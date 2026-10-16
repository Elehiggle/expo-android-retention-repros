# Expo monorepo checks

These checks supplement the small Fabric host and Android splash reproductions. They use Expo's actual `apps/bare-expo`, with no application backend or account.

Base: Expo `aaf153746ff4eba1f8399eff9b08e804114bf1c2`. Workspace versions: Expo 58.0.11, expo-modules-core 58.0.19, expo-splash-screen 58.0.9 and React Native 0.88.0. Package checks used Node 24.18.0 and pnpm 12.2.1. Native checks used JDK 17.0.20.1, NDK 27.1.12297006 and Android API 37 on x86_64.

## Package checks

Apply [core.patch](core.patch) and [splash.patch](splash.patch) to the pinned checkout, then run from its root:

```sh
pnpm install --frozen-lockfile --reporter=append-only
node ./tools/bin/expotools.js check-packages expo-modules-core expo-splash-screen
pnpm --filter expo-modules-core --filter expo-splash-screen run lint --fix
pnpm --filter expo-modules-core --filter expo-splash-screen run format
```

[Package results](package-results.json) record 16/16 checker tasks and 4/4 format tasks. Core passed 277 tests in 43 suites with 8 snapshots; Splash passed 16 tests in 8 suites. Eight build tasks used local cache; both Jest tasks executed. Existing Jest configuration and selected-workspace dependency-cycle warnings are recorded separately. Lint reported zero warnings. Neither candidate source nor the lockfile changed.

## Two alternative native fixtures

Both fixtures modify only bare-expo's Playground and benchmark-helper module. Apply exactly one to the pinned original files, after the production patches. Do not apply one fixture on top of the other. They are test code, not part of either production patch.

- [runtime-fixture-remount.patch](runtime-fixture-remount.patch) is the final fixture. A phase-based React key remounts the native view for each value. It produced the eight-row comparison below. Its Splash probe samples `onActivityPostDestroyed`, after destroy callbacks.
- [runtime-fixture-in-place.patch](runtime-fixture-in-place.patch) is the earlier fixture. It updates the same native view without a new key and preserves the failed live-update reproduction. Its older Splash callback runs in `onActivityDestroyed`; do not use it to reproduce the final post-destroy result.

[Runtime results](runtime-results.json) contain hashes for both fixtures, their original/resulting sources and both production patches, plus exact context identities and summarized observations. No raw logs, screenshots, APKs or heaps are included.

## Native commands and sequence

Follow Expo's Android setup instructions. From `apps/bare-expo/android`, both final variants were built with:

```sh
./gradlew :app:assembleDebug --no-daemon --console=plain \
  -PreactNativeDevServerPort=8093 \
  -PreactNativeArchitectures=x86_64 \
  -Pexpo.core.buildFromSource=true
```

Both builds exited 0 and compiled Core C++ through `:expo-modules-core:buildCMakeDebug[x86_64]`. An earlier successful build compiled patched Splash Kotlin; the final builds reported that task as `UP-TO-DATE`.

Run Metro for bare-expo on port 8093 with `EXPO_NO_METRO_LAZY=1 PROFILE=1`. With the target emulator selected, install and launch from the same Android directory:

```sh
adb install -r app/build/outputs/apk/debug/app-debug.apk
adb reverse tcp:8093 tcp:8093
adb shell am start -W -a android.intent.action.VIEW \
  -d 'bareexpo://?__expo_url=http%3A%2F%2Flocalhost%3A8093' \
  -n dev.expo.payments/.MainActivity
```

Open Playground. The dark React text shows expected values; the gray native TextView shows actual values. Its `label` uses an ordinary prop. Its `values` prop takes `List<Int>` and is marked `.asStateProp()`.

1. Change values through `[1, 2]`, `[3, 5, 8]` and `[]`. Read the native TextView, not just React's expected text.
2. Reload the React runtime and reopen Playground. Record the process ID, JS nonce, module instance, AppContext and ReactContext. Repeat three times.
3. In the candidate, call the three Splash options, recreate the Android Activity, reopen Playground and refresh its lifecycle snapshot. Use the final remount fixture for this check.
4. Repeat the Fabric sequence with original Core sources, keeping patched Splash and the same fixture. Rebuild Core from source. This is the baseline; a prebuilt Core AAR is not a substitute.
5. To reproduce the earlier in-place failures, use a separate clean copy of the original fixture files, apply the in-place fixture instead, then build and compare both Core variants.

## Results and limits

The final remount matrix records native values. `unset` means no list was delivered; `[]` is a valid delivered empty list. Generation 0 is the initial runtime; generations 1–3 follow real JS reloads. Exact process IDs, JS nonces and native context IDs for all eight rows are in [runtime-results.json](runtime-results.json).

| Core variant | Generation | Phase 1: expected `[3, 5, 8]` | Phase 2: expected `[]` |
| --- | ---: | --- | --- |
| Candidate | 0 | `[3, 5, 8]` | `unset` |
| Candidate | 1 | `unset` | `[]` |
| Candidate | 2 | `[3, 5, 8]` | `[]` |
| Candidate | 3 | `unset` | `unset` |
| Original baseline | 0 | `[3, 5, 8]` | `[]` |
| Original baseline | 1 | `[3, 5, 8]` | `unset` |
| Original baseline | 2 | `[3, 5, 8]` | `[]` |
| Original baseline | 3 | `unset` | `[]` |

**State-prop delivery failed in both variants.** The candidate delivered 2/4 values per phase; the baseline delivered 3/4. These rates differ. Four generations per variant do not establish equivalence or isolate a candidate-specific regression. This is not a green converter/update suite.

The earlier in-place fixture also failed in both variants: candidate phase 1 expected `[3, 5, 8]` and phase 2 expected `[]`, but the native list stayed `[1, 2]`; baseline phase 2 likewise stayed `[1, 2]`. The native ordinary-prop label tracked the phase while the list did not. Keyed remounts do not validate live updates.

Both final variants completed three genuine runtime reloads. Each kept its Android process and changed its JS nonce, module instance, AppContext and ReactContext. Both logged the same non-fatal UI-thread soft assertion in `reloadAppAsync` and ReactHost teardown SoftExceptions. Neither process had a recorded fatal crash; these were not zero-warning runs.

The separate candidate Splash check passed once: after three `setOptions` calls and one Activity recreation, `onActivityPostDestroyed` observed the old splash owner cleared, and the new Activity registered an owner. This is a field/lifecycle observation, not proof of full-app collection.

The host tests remain separate evidence for provider/descriptor counts, schema snapshots and isolated Splash Activity collection. This monorepo run measures neither retained bytes nor a memory plateau, and covers only API 37 x86_64. No cloud build or GitHub workflow ran.
