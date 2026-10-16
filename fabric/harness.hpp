// MIT. Test doubles for the platform APIs surrounding the extracted upstream functions.
#pragma once
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

inline void check(bool ok, const char *message) {
  if (!ok) { std::cerr << message << '\n'; std::exit(1); }
}
#define react_native_assert(value) check(value, #value)

namespace facebook::jni {
template<class T> using alias_ref = std::shared_ptr<T>;
struct JString { std::string value; std::string toStdString() const { return value; } };
template<class T> struct JArrayClass {
  std::vector<std::shared_ptr<T>> values;
  size_t size() const { return values.size(); }
  auto getElement(size_t i) const { return values.at(i); }
};
}
namespace facebook::react {
using ComponentHandle = std::uintptr_t;
using ComponentName = const char *;
struct ComponentDescriptorParameters {
  std::shared_ptr<int> eventDispatcher;
  std::shared_ptr<int> contextContainer;
  std::shared_ptr<const void> flavor;
};
struct RawPropsParser { explicit RawPropsParser(bool) {} };
struct ComponentDescriptor {
  using Flavor = std::shared_ptr<const void>;
  using Unique = std::unique_ptr<ComponentDescriptor>;
  explicit ComponentDescriptor(const ComponentDescriptorParameters &p) : flavor(std::static_pointer_cast<const std::string>(p.flavor)) {}
  virtual ~ComponentDescriptor() = default;
  ComponentHandle getComponentHandle() const { return reinterpret_cast<ComponentHandle>(flavor->c_str()); }
  ComponentName getComponentName() const { return flavor->c_str(); }
  std::shared_ptr<const std::string> flavor;
};
struct ComponentDescriptorProvider {
  ComponentHandle handle;
  ComponentName name;
  ComponentDescriptor::Flavor flavor;
  ComponentDescriptor::Unique (*constructor)(const ComponentDescriptorParameters &);
};
struct ComponentDescriptorProviderRegistry;
struct ComponentDescriptorRegistry {
  using Shared = std::shared_ptr<const ComponentDescriptorRegistry>;
  ComponentDescriptorRegistry(ComponentDescriptorParameters p, const ComponentDescriptorProviderRegistry &, const std::shared_ptr<int> &) : parameters_(std::move(p)) {}
  void add(const ComponentDescriptorProvider &) const;
  ComponentDescriptorParameters parameters_;
  mutable std::shared_mutex mutex_;
  mutable std::unordered_map<ComponentHandle, std::shared_ptr<const ComponentDescriptor>> _registryByHandle;
  mutable std::unordered_map<std::string, std::shared_ptr<const ComponentDescriptor>> _registryByName;
};
struct ComponentDescriptorProviderRegistry {
  void add(const ComponentDescriptorProvider &) const;
  ComponentDescriptorRegistry::Shared createComponentDescriptorRegistry(const ComponentDescriptorParameters &) const;
  mutable std::shared_mutex mutex_;
  mutable std::unordered_map<ComponentHandle, ComponentDescriptorProvider> componentDescriptorProviders_;
  mutable std::vector<std::weak_ptr<const ComponentDescriptorRegistry>> componentDescriptorRegistries_;
};
struct CoreComponentsRegistry {
  static auto sharedProviderRegistry() {
    static auto registry = std::make_shared<ComponentDescriptorProviderRegistry>();
    return registry;
  }
};
}
namespace jni = facebook::jni;
namespace react = facebook::react;
namespace expo {
struct ExpectedType { int generation; };
class FrontendConverter {
public:
  explicit FrontendConverter(int generation) : generation(generation) {}
  int generation;
};
class FrontendConverterProvider {
public:
  static auto instance() { return std::make_shared<FrontendConverterProvider>(); }
  auto obtainConverter(const std::shared_ptr<ExpectedType> &type) {
    return std::make_shared<FrontendConverter>(type->generation);
  }
};
class FabricComponentsRegistry {
public:
  void registerComponentsRegistry(
    jni::alias_ref<jni::JArrayClass<jni::JString>>,
    jni::alias_ref<jni::JArrayClass<jni::JArrayClass<jni::JString>>>,
    jni::alias_ref<jni::JArrayClass<jni::JArrayClass<ExpectedType>>>);
};
using Schema = std::unordered_map<std::string, std::shared_ptr<FrontendConverter>>;
}
