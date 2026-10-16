#include "ExpoViewComponentRegistry.h"

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <unordered_set>
#include <vector>

// Registry ownership depends on shared_ptr, not JNI conversion. Use a tiny converter
// to observe release while compiling the actual patched registry header unchanged.
namespace expo {
class FrontendConverter {
public:
  explicit FrontendConverter(int value) : value(value) {}
  int value;
};
}

static void check(bool condition, const char *message) {
  if (!condition) {
    std::cerr << message << '\n';
    std::abort();
  }
}

static std::shared_ptr<expo::FrontendConverter> converter(int value) {
  return std::make_shared<expo::FrontendConverter>(value);
}

int main() {
  expo::ExpoViewComponentRegistry registry;
  auto firstConverter = converter(1);
  std::weak_ptr<expo::FrontendConverter> releasedConverter = firstConverter;
  auto registration = registry.registerComponent("ExpoExample", {{"oldProp", firstConverter}});
  firstConverter.reset();
  const auto originalName = registration->flavor->c_str();
  auto inFlight = registration->getStateProps();
  std::weak_ptr<const expo::ExpoViewStateProps> releasedSnapshot = inFlight;

  std::unordered_set<std::uintptr_t> handles;
  for (int reload = 0; reload < 1000; ++reload) {
    auto current = registry.registerComponent("ExpoExample", {{"newProp", converter(reload)}});
    handles.insert(reinterpret_cast<std::uintptr_t>(current->flavor->c_str()));
    check(current == registration, "reload must update the existing registration");
    check(current->flavor->c_str() == originalName, "component name pointer must survive reload");
    auto latest = registration->getStateProps();
    check(latest->size() == 1 && latest->contains("newProp"), "schema changes must add and remove props");
    check(latest->at("newProp")->value == reload, "existing descriptors must see the new converter");
    check(inFlight->size() == 1 && inFlight->at("oldProp")->value == 1, "an in-flight render needs one unchanged schema");
  }
  check(handles.size() == 1, "RN must receive one handle for one component name across reloads");
  check(!releasedConverter.expired(), "the in-flight snapshot must retain its converter");
  inFlight.reset();
  check(releasedSnapshot.expired() && releasedConverter.expired(), "old schemas must release after their final reader");

  auto late = registry.registerComponent("ExpoLateModule", {{"lateProp", converter(42)}});
  check(late != registration && late->flavor->c_str() != originalName, "new module names need distinct handles");
  check(registry.find("ExpoLateModule") == late, "late registration must remain discoverable");
  check(registry.find("Missing") == nullptr, "an unknown component must not create a registration");
  registry.registerComponent("ExpoExample", {});
  check(registration->getStateProps()->empty(), "a replacement empty schema must remove previous props");
  auto restored = registry.registerComponent("ExpoExample", {{"oldProp", converter(2)}});
  check(restored == registration && restored->flavor->c_str() == originalName, "A to B to empty to A must preserve identity");
  check(restored->getStateProps()->size() == 1 && restored->getStateProps()->at("oldProp")->value == 2, "restoring a prior schema must publish its new converter");

  auto concurrent = registry.registerComponent("Concurrent", {{"left", converter(0)}, {"right", converter(0)}});
  const auto concurrentName = concurrent->flavor->c_str();
  std::atomic<bool> start{false};
  std::vector<std::thread> workers;
  std::vector<std::shared_ptr<expo::ExpoViewComponentRegistration>> firstRegistrations(8);
  for (int worker = 0; worker < 8; ++worker) {
    workers.emplace_back([&, worker] {
      while (!start.load(std::memory_order_acquire)) {
        std::this_thread::yield();
      }
      firstRegistrations[worker] = registry.registerComponent("ConcurrentFirst", {{"prop", converter(worker)}});
      for (int iteration = 0; iteration < 2000; ++iteration) {
        if (worker < 4) {
          const int value = worker * 2000 + iteration;
          auto current = registry.registerComponent("Concurrent", {{"left", converter(value)}, {"right", converter(value)}});
          check(current == concurrent && current->flavor->c_str() == concurrentName, "concurrent writers must preserve identity");
        } else {
          auto current = registry.find("Concurrent");
          check(current == concurrent, "readers must see the existing registration");
          auto snapshot = current->getStateProps();
          check(snapshot->size() == 2, "a reader must not observe a partial schema");
          check(snapshot->at("left")->value == snapshot->at("right")->value, "filter and conversion must use one schema generation");
        }
      }
    });
  }
  start.store(true, std::memory_order_release);
  for (auto &worker : workers) {
    worker.join();
  }
  for (const auto &first : firstRegistrations) {
    check(first == firstRegistrations.front(), "racing first registrations must publish exactly one identity");
  }
  std::cout << "Expo registry identity, metadata replacement, release and concurrent snapshots passed\n";
}
