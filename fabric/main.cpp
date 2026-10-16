// The generated translation unit defines the upstream registration functions before this file.
int main() {
  expo::FabricComponentsRegistry registrar;
  auto names = std::make_shared<jni::JArrayClass<jni::JString>>();
  names->values.push_back(std::make_shared<jni::JString>(jni::JString{"ExpoReproView"}));
  auto keys = std::make_shared<jni::JArrayClass<jni::JArrayClass<jni::JString>>>();
  auto key = std::make_shared<jni::JArrayClass<jni::JString>>();
  key->values.push_back(std::make_shared<jni::JString>(jni::JString{"value"}));
  keys->values.push_back(key);
  auto types = std::make_shared<jni::JArrayClass<jni::JArrayClass<expo::ExpectedType>>>();
  auto type = std::make_shared<jni::JArrayClass<expo::ExpectedType>>();
  type->values.push_back(std::make_shared<expo::ExpectedType>(expo::ExpectedType{0}));
  types->values.push_back(type);
  auto providers = react::CoreComponentsRegistry::sharedProviderRegistry();
  auto liveRegistry = providers->createComponentDescriptorRegistry({});
  std::shared_ptr<const expo::AndroidExpoViewComponentDescriptor> firstDescriptor;
  std::shared_ptr<const expo::Schema> inFlight;
  std::weak_ptr<expo::FrontendConverter> firstConverter;
  constexpr int registrations = 1000;
  for (int generation = 0; generation < registrations; ++generation) {
    type->values.front()->generation = generation;
    registrar.registerComponentsRegistry(names, keys, types);
    if (generation == 0) {
      firstDescriptor = std::dynamic_pointer_cast<const expo::AndroidExpoViewComponentDescriptor>(liveRegistry->_registryByName.at("ExpoReproView"));
      inFlight = firstDescriptor->snapshot();
      firstConverter = inFlight->at("value");
    }
  }
  auto lateRegistry = providers->createComponentDescriptorRegistry({});
  const auto count = providers->componentDescriptorProviders_.size();
  std::cout << "registrations=" << registrations << " providers=" << count
            << " live_descriptors=" << liveRegistry->_registryByHandle.size()
            << " late_descriptors=" << lateRegistry->_registryByHandle.size()
            << " first_descriptor_schema_generation=" << firstDescriptor->snapshot()->at("value")->generation << '\n';
#ifdef FIXED
  check(count == 1, "one name must keep one provider");
  check(liveRegistry->_registryByHandle.size() == 1, "live registries must not accumulate descriptors");
  check(lateRegistry->_registryByHandle.size() == 1, "new registries must not rebuild old duplicates");
  check(firstDescriptor->snapshot()->at("value")->generation == registrations - 1, "cached descriptor must read the current schema");
  check(inFlight->at("value")->generation == 0, "in-flight snapshot must not change");
  inFlight.reset();
  check(firstConverter.expired(), "obsolete converter must release after its last reader");
#else
  check(count == registrations, "baseline should reproduce unique handles for repeated names");
  check(expo::statePropMap.size() == registrations, "baseline should retain one schema per registration");
  check(liveRegistry->_registryByHandle.size() == registrations, "baseline live registry must receive duplicates");
  check(lateRegistry->_registryByHandle.size() == registrations, "baseline new registry must rebuild duplicates");
  check(firstDescriptor->snapshot()->at("value")->generation == 0, "baseline old descriptor keeps its old schema");
#endif
  std::cout << "PASS\n";
}
