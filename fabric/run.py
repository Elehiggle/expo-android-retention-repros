#!/usr/bin/env python3
"""Compile exact upstream registration function bodies with narrow platform doubles."""
import hashlib, json, os, pathlib, subprocess
ROOT=pathlib.Path(__file__).resolve().parent
manifest=json.loads((ROOT/'sources.json').read_text())
for name, info in manifest['files'].items():
 p=ROOT/name
 if hashlib.sha256(p.read_bytes()).hexdigest()!=info['sha256']:
  raise SystemExit('Source checksum changed: '+name)

def function(source, prefix):
 start=source.index(prefix); brace=source.index('{',start); depth=1;end=brace+1
 while depth:
  depth += (source[end]=='{')-(source[end]=='}');end+=1
 return source[start:end]

rn_provider=(ROOT/'sources/react-native/ComponentDescriptorProviderRegistry.cpp').read_text()
rn_descriptor=(ROOT/'sources/react-native/ComponentDescriptorRegistry.cpp').read_text()
provider_add=function(rn_provider,'void ComponentDescriptorProviderRegistry::add(')
provider_create=function(rn_provider,'ComponentDescriptorRegistry::Shared\nComponentDescriptorProviderRegistry::createComponentDescriptorRegistry(')
descriptor_add=function(rn_descriptor,'void ComponentDescriptorRegistry::add(')
build=ROOT/'build';build.mkdir(exist_ok=True)
results=[]
for variant in ['expo-57.0.18','expo-57.0.23','expo-58.0.19','expo-main','fixed']:
 source=ROOT/'sources'/variant
 factory=(source/'ExpoComponentDescriptorFactory.cpp').read_text()
 register=(source/'FabricComponentsRegistry.cpp').read_text()
 descriptor=(source/'AndroidExpoViewComponentDescriptor.cpp').read_text()
 fixed=variant=='fixed'
 header='#include "harness.hpp"\n'
 if fixed:
  header+='#define FIXED 1\n#include "sources/fixed/ExpoViewComponentRegistry.h"\n'
 declarations='namespace expo {\n'
 if fixed:
  declarations+='ExpoViewComponentRegistry &getExpoViewComponentRegistry();\n'
 else:
  declarations+='using StatePropMapType = std::unordered_map<react::ComponentDescriptor::Flavor, Schema>;\nStatePropMapType statePropMap;\nstd::shared_mutex statePropMapMutex;\n'
 declarations+='''class AndroidExpoViewComponentDescriptor : public react::ComponentDescriptor {
public:
  AndroidExpoViewComponentDescriptor(const react::ComponentDescriptorParameters &p, react::RawPropsParser) : react::ComponentDescriptor(p) {}
'''
 if fixed:
  declarations+='''  void setComponentRegistration(std::shared_ptr<ExpoViewComponentRegistration> registration);
  std::shared_ptr<const Schema> snapshot() const { return registration_->getStateProps(); }
private:
  std::shared_ptr<ExpoViewComponentRegistration> registration_;
};
'''
  declarations+=function(factory,'ExpoViewComponentRegistry &getExpoViewComponentRegistry()')+'\n'
  declarations+=function(descriptor,'void AndroidExpoViewComponentDescriptor::setComponentRegistration(')+'\n'
 else:
  declarations+='''  void setStateProps(const std::unordered_map<std::string, std::shared_ptr<FrontendConverter>> &stateProps);
  std::shared_ptr<const Schema> snapshot() const { return std::make_shared<const Schema>(stateProps_); }
private:
  Schema stateProps_;
};
'''
  declarations+=function(descriptor,'void AndroidExpoViewComponentDescriptor::setStateProps(')+'\n'
 declarations+=function(factory,'react::ComponentDescriptor::Unique concreteExpoComponentDescriptorConstructor(')+'\n'
 declarations+=function(register,'void FabricComponentsRegistry::registerComponentsRegistry(')+'\n}\n'
 unit=header+declarations+'\nnamespace facebook::react {\n'+provider_add+'\n'+provider_create+'\n'+descriptor_add+'\n}\n#include "main.cpp"\n'
 cpp=build/(variant+'.cpp');cpp.write_text(unit)
 binary=build/variant
 command=[os.environ.get('CXX','c++'),'-std=c++20','-O2','-DNDEBUG','-Wall','-Wextra','-Werror','-pthread','-I',str(ROOT),str(cpp),'-o',str(binary)]
 subprocess.run(command,check=True)
 result=subprocess.run([str(binary)],text=True,capture_output=True,check=True,timeout=15)
 print(variant+': '+result.stdout.strip().replace('\n','; '),flush=True)
 results.append({'variant':variant,'compileCommand':command,'exitCode':result.returncode,'stdout':result.stdout,'generatedSourceSha256':hashlib.sha256(unit.encode()).hexdigest()})
# Exercise the actual new header directly, including concurrent first registration
# and schema readers/writers. This test does not replace the source-level repro above.
command=[os.environ.get('CXX','c++'),'-std=c++20','-O2','-DNDEBUG','-Wall','-Wextra','-Werror','-pthread','-I',str(ROOT/'sources/fixed'),str(ROOT/'registry-test.cpp'),'-o',str(build/'registry-test')]
subprocess.run(command,check=True)
result=subprocess.run([str(build/'registry-test')],text=True,capture_output=True,check=True,timeout=15)
print(result.stdout.strip())
results.append({'variant':'registry-concurrency','compileCommand':command,'exitCode':result.returncode,'stdout':result.stdout})
(build/'results.json').write_text(json.dumps(results,indent=2)+'\n')
