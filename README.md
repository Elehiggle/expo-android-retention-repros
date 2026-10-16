# Expo Android retention repros

Two small reproductions for separate native ownership issues in Expo:

- **[Fabric registration](fabric/):** registering the same native view again creates another pointer-based component handle. React Native retains the extra provider and descriptors. The proposed fix keeps one registration per name and replaces its state-prop schema.
- **[Splash Activity retention](splash/):** the splash manager and platform exit listener can keep a destroyed Activity reachable. The proposed fix clears the listener and releases only the Activity's own manager reference.

The source versions are pinned to Expo commit `aaf153746ff4eba1f8399eff9b08e804114bf1c2`. The supplied before/after sources and manifests identify the exact changes. These are separate defects; a combined patched build does not measure each fix's memory effect.

## Run

**Fabric source-level test:** requires Python 3 and a C++20 compiler. It runs offline.

```sh
cd fabric
python3 run.py
```

**Native splash test:** requires Python 3, JDK 17+, Android SDK 36, and an Android device or emulator. The checked-in Gradle wrapper downloads its pinned distribution and Android dependencies. See [splash/README.md](splash/README.md) for the before/after commands and result interpretation.

**Ordinary Expo app:** requires Node/npm, Python 3, JDK, Android SDK/NDK, and a device or emulator. See [expo-app/README.md](expo-app/README.md). It builds both packages from source and gives each fix an independent before/after selector. It uses no backend or account.

**Expo monorepo checks:** the pinned `apps/bare-expo` package/build/reload procedure and exact fixture are in [monorepo-checks/README.md](monorepo-checks/README.md).

## Results and limits

The Fabric host test reproduces 1,000 providers and descriptors after 1,000 registrations in Expo 57.0.18, 57.0.23, 58.0.19 and the pinned main source. The candidate keeps each count at one. It also checks schema freshness, converter release and concurrent access. See [fabric/expected-results.json](fabric/expected-results.json) and the [test boundaries](fabric/README.md#limits).

The native splash test ran on Android API 37. The original manager retained the destroyed owner and Activity; the candidate passed all five checks. See [splash/RESULTS.md](splash/RESULTS.md).

The ordinary Expo 58 app compiled both candidates from source (core C++ for four ABIs and splash Kotlin). On Android API 37, the native gradient updated before and after three Activity recreates in the same process. Repeated splash options and finishing the Activity also passed. See [expo-app/validation.json](expo-app/validation.json).

The later monorepo run passed the official package checks (277 Core tests and 16 Splash tests), lint and formatting. Original and patched Core each completed three genuine JS reloads in `apps/bare-expo`. Splash owner cleanup and re-registration passed. The native state-prop probe failed live updates and intermittently lost remount values in **both** Core variants; it is not a passing converter test. The complete comparison and shared nonfatal reload warnings are recorded in [monorepo-checks/runtime-results.json](monorepo-checks/runtime-results.json).

Source: [Elehiggle/expo-android-retention-repros](https://github.com/Elehiggle/expo-android-retention-repros).

The host test does not render views or measure Android memory in bytes. The native splash test exercises real Activities and AndroidX while replacing React Native startup inputs; the Expo app covers integration separately. These checks do not establish a production memory plateau or behavior on every Android version.

## Provenance and licenses

Original Expo and React Native source keeps its MIT license files. The Gradle wrapper keeps its Apache-2.0 license and recorded source hashes. The source manifests include upstream commits, release versions and checksums. Small test results contain counts and outcomes only.

No application data, heap dumps, credentials, CI workflows, EAS configuration, dependency directories or build outputs belong in this repository. All builds described here run locally.
