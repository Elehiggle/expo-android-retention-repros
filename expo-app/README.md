# Minimal Expo 58 app

An ordinary Expo app for local native integration checks. It has no backend, account, private app content, EAS configuration or GitHub workflow. The dependency lock is checked in. `expo-splash-screen` 58.0.9 and `expo-modules-core` 58.0.19 both build from source.

## Build

Requires Node/npm, Python 3, JDK 17+, Android SDK/NDK and a device or emulator. The completed source-build check used Java 17, Android platform/build-tools 37 and NDK 27.1.12297006. Prebuild generates the Android project and its pinned Gradle wrapper; generated Android files and build outputs are not checked in here. The command below selects the embedded template from the lock-pinned Expo package explicitly. Its [version and checksum](prebuild-template.json) identify template 58.0.20 and Gradle 9.4.1. The recorded build used the CLI default, which tries that embedded template first; it did not pass an explicit template flag.

```sh
npm ci
python3 select-source.py before
python3 select-core-source.py before
EXPO_NO_METRO_LAZY=1 PROFILE=1 npx expo prebuild --platform android --no-install --template ./node_modules/expo/template.tgz
npm run android
```

This builds locally and starts Metro on port 8093. Do not use Expo Go. Each source selector validates the installed package version and expected source before writing. `select-core-source.py` also checks the complete before/after file manifest.

For a compile check without launching a device or Metro:

```sh
cd android
./gradlew :app:assembleDebug --no-daemon --console=plain -PreactNativeDevServerIp=localhost -PreactNativeDevServerPort=8093
```

## Splash before/after

Keep core on `before` for both runs to isolate the splash change. After a fresh launch, wait for the main screen. Run this three times, waiting for the screen to return each time:

```sh
adb -s "$ANDROID_SERIAL" shell am start -n org.example.splashretentionexpo/.MainActivity --activity-single-top -a org.example.splashretentionexpo.RECREATE
```

Then finish the last Activity without stopping the process:

```sh
adb -s "$ANDROID_SERIAL" shell am start -n org.example.splashretentionexpo/.MainActivity --activity-single-top -a org.example.splashretentionexpo.FINISH
```

The local config plugin adds these two actions to the generated Activity. In Android Studio, request GC and inspect destroyed MainActivity instances and incoming references. The reported paths go through `SplashScreenManager.splashScreen` and the platform exit listener. Keep dumps local.

Select the candidate, rebuild, then repeat in a fresh process:

```sh
python3 select-source.py after
python3 select-core-source.py before
npm run android
```

The smaller [native splash host](../splash/) supplies automated owner/lifecycle checks without a complete RN runtime. This app adds ordinary Expo integration. Cold launch, repeated options, background/foreground behavior and older-owner cleanup are separate checks; a successful build alone does not establish them.

## Fabric candidate

To compile both native candidates together:

```sh
python3 select-source.py after
python3 select-core-source.py after
npm run android
```

Do not attribute a memory difference between a combined before/after build to one candidate. The [Fabric source-level test](../fabric/) isolates registration counts and schema lifetime. The screen mounts a real `expo-linear-gradient@58.0.3` native Expo view. Tap `gradient-colors-toggle` to change red/yellow to blue/cyan and back, then repeat after each Activity recreation. This checks visible native prop updates; it does not count retained providers or exercise every possible state-prop converter.

## Validation

The checked-in [validation.json](validation.json) records source hashes and completed checks. The combined after-source build passed installation, Android prebuild and local Gradle `assembleDebug`. Expo core C++ compiled from source for arm64-v8a, armeabi-v7a, x86 and x86_64; the patched splash manager compiled as Kotlin. The first build attempt stopped because SDK37 components were missing; after installing them, the second build passed.

On an Android API 37 x86_64 emulator, cold launch and gradient color changes passed. After three Activity recreates in the same process, gradient changes still worked. Three repeated splash-options calls completed, and `FINISH` removed the Activity while the process remained alive. No fatal exception or RN JavaScript error appeared. One initial packager WebSocket retry warning occurred before the bundle loaded. No full-app heap measurement or Android 12–13 test is claimed. This receipt describes the earlier ordinary-app smoke. The later official package checks and `apps/bare-expo` reload comparison are recorded separately in [monorepo-checks](../monorepo-checks/README.md), including the failing state-prop probe in both variants. For an upstream issue, run `npx expo-env-info` and `npx expo-doctor@latest` in this directory and report their actual output.
