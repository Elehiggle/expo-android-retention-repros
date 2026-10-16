# Android splash Activity retention

This native Android app compiles `SplashScreenManager.kt` from Expo commit
`aaf153746ff4eba1f8399eff9b08e804114bf1c2`. The file is byte-identical to the
one published in `expo-splash-screen@58.0.9`.

It starts and finishes real Activities. It uses AndroidX `core-splashscreen:1.2.0`
and the device's real platform splash implementation. There is no JavaScript,
Metro, backend, login, EAS build, GitHub Actions workflow, or application data.

The host replaces two inputs which do not implement the behavior under test:

- `ReactMarker` stores the listener but does not start React Native. The native
  manager's `hide()` method releases the pre-draw gate directly.
- `SplashScreenOptions` supplies the same default `duration` and `fade` fields.
  It omits Expo's record conversion and annotations.

The actual manager, AndroidX implementation, Android Activity lifecycle, exit
listener registration, and collection checks run without mocks. This isolates
native ownership; it does not test Expo autolinking, JS record conversion, a
complete React Native runtime, or every Android release. `../expo-app` provides
an additional ordinary Expo app for those integration checks.

## Run before and after

Requires JDK 17+, Android SDK 36, and an Android device or emulator. The Gradle
wrapper pins Gradle 9.4.1 with its official distribution checksum. Build the
APK locally; do not use EAS or a hosted workflow.

Run from this directory. Set `ANDROID_HOME` to the local SDK and choose the
existing device's serial with `ANDROID_SERIAL`.

```bash
python3 select-source.py before
./gradlew :app:assembleDebug --no-daemon --console=plain
adb -s "$ANDROID_SERIAL" install -r app/build/outputs/apk/debug/app-debug.apk
adb -s "$ANDROID_SERIAL" shell am force-stop org.example.splashretention
adb -s "$ANDROID_SERIAL" shell am instrument -w org.example.splashretention/.SplashRetentionInstrumentation

python3 select-source.py after
./gradlew :app:assembleDebug --no-daemon --console=plain
adb -s "$ANDROID_SERIAL" install -r app/build/outputs/apk/debug/app-debug.apk
adb -s "$ANDROID_SERIAL" shell am force-stop org.example.splashretention
adb -s "$ANDROID_SERIAL" shell am instrument -w org.example.splashretention/.SplashRetentionInstrumentation
```

`am instrument` can exit zero for a failed assertion. Read its `result` field
and `INSTRUMENTATION_CODE`: this runner reports `result=PASS` and code `-1`
only when all checks pass. Preserve both transcripts. The unpatched run should
report a retained owner and Activity. The patched run should report `PASS` for
headless options, newer-owner protection, destroyed-owner release, destroyed-
Activity collection, and options after destruction.

The runner reports its current phase and has a 60-second overall deadline.
It starts each instance in a new task so Android cannot reuse the existing root
Activity while instrumentation waits for a new one. All tasks use the same app
process. The runner waits for each real `onDestroy`. The Activity calls `setOptions`
three times after registration. It creates a newer Activity before finishing an
older one, to check that old cleanup cannot clear the new owner. The collection
check allows two seconds of explicit GC attempts; it reports failure if the
Activity remains reachable, rather than treating a missing collection as proof
of success. Keep heap profilers and debuggers detached during the check.

The current source selection is in `app/src/main/java/expo/modules/splashscreen/`.
`select-source.py` prints the selected source hash. Before source SHA-256:
`aea45b799b67f02af0aa917873dc0e5d647d3bae793eae9be21841a417edbaec`.

## Manual inspection

Launch the app, open the splash Activity, then finish it. Repeat several times.
In Android Studio, record a Java heap after GC. Inspect destroyed `SplashActivity`
instances and their incoming references. On the unpatched source the relevant
paths go through the manager's static `splashScreen` and the platform splash
listener. The button label is only a convenience; instrumentation drives the
Activity lifecycle directly.

## Status

Built and run on an Android API 37 emulator on 2026-10-16. The unchanged manager
retained its destroyed owner and Activity; the candidate passed all five checks.
See [RESULTS.md](RESULTS.md) for exact results, APK hashes, and limits.
No upstream issue, pull request, fork, or workflow was created by this package.
