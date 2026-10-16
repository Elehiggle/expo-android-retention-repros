# Repeated Expo Fabric registration on Android

This source-level reproducer registers `ExpoReproView` 1,000 times in one process. It checks how many providers and descriptors React Native retains, both in a registry that already exists and in one created after the registrations.

The package contains no app code, account data, Android project, dependencies to install, or GitHub workflows. It runs offline with Python 3 and a C++20 compiler:

```sh
python3 run.py
# Optional: CXX=clang++ python3 run.py
```

The script verifies the vendored source checksums before compiling. It uses `-O2 -DNDEBUG -Wall -Wextra -Werror -pthread`; its checks remain active when C assertions are disabled. Generated files stay under `build/`.

Expected results:

| Expo source | Registrations | Providers | Existing registry descriptors | New registry descriptors |
| --- | ---: | ---: | ---: | ---: |
| 57.0.18 | 1,000 | 1,000 | 1,000 | 1,000 |
| 57.0.23 | 1,000 | 1,000 | 1,000 | 1,000 |
| 58.0.19 | 1,000 | 1,000 | 1,000 | 1,000 |
| main at `aaf153746ff4eba1f8399eff9b08e804114bf1c2` | 1,000 | 1,000 | 1,000 | 1,000 |
| proposed fix on that main | 1,000 | 1 | 1 | 1 |

All four baseline runs deliberately check that the defect reproduces. `PASS` means that the expected counts were observed, not that the baseline is free of the defect. The fixed run additionally checks that an existing descriptor sees the latest schema while an in-flight reader retains its original snapshot. The old converter is released when that reader finishes.

## What executes

`run.py` extracts these unchanged function definitions from the vendored source and compiles them into each test executable:

- Expo `FabricComponentsRegistry::registerComponentsRegistry`.
- Expo `concreteExpoComponentDescriptorConstructor`.
- Expo `AndroidExpoViewComponentDescriptor::setStateProps`, or `setComponentRegistration` in the fix.
- React Native `ComponentDescriptorProviderRegistry::add` and `createComponentDescriptorRegistry`.
- React Native `ComponentDescriptorRegistry::add`.

The fixed run also compiles the complete proposed `ExpoViewComponentRegistry.h` unchanged. `registry-test.cpp` separately exercises that header with 1,000 reloads, a new name, empty/replaced schemas, converter release, concurrent first registration, and eight threads reading/writing complete schema snapshots.

The Expo main SHA is pinned above. The React Native definitions come from 0.88.0, commit `5c35c82cf7dd322bf94d47501508c3de975c077b`. The same RN definitions are used for all variants to isolate the Expo registration change; this is not an SDK 57 runtime compatibility test. Package source came from the public npm release tarballs. `sources.json` records each file's checksum and origin; `releases.json` records tarball checksums and npm metadata. MIT license files are included.

## Limits

`harness.hpp` supplies small JNI string/array, converter, descriptor, and registry declarations. A converter holds a generation number. A descriptor exposes its name and schema, but does not create a real `RawPropsParser`, JSI runtime, Java reference, Android view, or Fabric surface. Registry storage uses the same handle/name keys as RN, and the upstream add/create bodies perform the real registration logic.

This demonstrates duplicate handles, provider/descriptor retention, and schema lifetime at the registration boundary. It does not measure bytes or prove an Android memory plateau, render correctness, Gradle/NDK compatibility, or all runtime teardown behavior. The later [bare-expo integration probe](../monorepo-checks/README.md) exercised native state props across JS reloads, but delivery failed in both original and patched Core. It does not establish a fully working rendering path. The proposed patch keeps one schema snapshot through filtering, serializable-state initialization, and conversion; the host reproducer does not execute those rendering steps.

The separate race fix in [Expo #51044](https://github.com/expo/expo/pull/51044) already protects the old map. That PR's [verification comment](https://github.com/expo/expo/pull/51044#issuecomment-5996913171) also notes the remaining per-registration entries. This reproducer covers that remaining ownership issue.
