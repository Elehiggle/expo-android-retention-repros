# Native verification — 2026-10-16

Both APKs built locally and ran on the same Android API 37 emulator. The test
used the source and commands described in README.md. The manager before source
matches Expo main `aaf153746ff4eba1f8399eff9b08e804114bf1c2` and npm 58.0.9.

| Check | Before | After |
| --- | --- | --- |
| Options before any Activity exists | PASS | PASS |
| Older Activity cannot clear newer owner | PASS | PASS |
| Destroyed Activity's global splash owner is cleared | FAIL | PASS |
| Destroyed Activity is collected after repeated setOptions | FAIL | PASS |
| Options after destruction | Not reached after the expected failure | PASS |

The before runner reported `result=FAIL`, `INSTRUMENTATION_CODE: 0`. The after
runner reported `result=PASS`, `INSTRUMENTATION_CODE: -1`. Both Gradle builds
exited zero. The instrumentation shell command itself exits zero in both cases;
that is not the assertion result.

APK SHA-256:

- Before: `6ae64cec03e7ed77e4f4a00cab882a265110e5d638b6241a6ccc83474f6ed873`
- After: `0eea41445583947e842e49390992cae3f7a58b508276eb4b84de64efab662d04`

These checks run real Android Activities, AndroidX 1.2.0, and the platform splash
implementation. The host replaces ReactMarker and Expo options conversion; it
does not start React Native. It proves release of the tested owner and Activity
on API 37. It does not measure process memory, test all Android versions, or
prove a full Expo app integration. No screenshots are needed: this change has
no intended UI effect.
