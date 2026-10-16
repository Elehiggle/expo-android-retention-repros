# Package checks — 2026-10-16

- The Fabric test ran again from the assembled `fabric/` directory. All four original-source variants reproduced 1,000 providers/descriptors; the candidate retained one. The registry-header concurrency test passed. Results match `fabric/expected-results.json`.
- Every vendored Fabric source, Expo core selector source and Gradle wrapper file matches its recorded SHA-256. The final Expo app lock, App.js and app.json match the completed native build/runtime receipt.
- All JSON parses, all Python files pass syntax parsing, and local Markdown links resolve. The npm lock resolves packages through the public npm registry.
- The package contains no dependency directories, build outputs, APKs, heaps, raw device logs, private filesystem paths, credentials, GitHub workflows or EAS configuration. The only checked-in JAR is the official Gradle wrapper, with provenance and its license.
- The native splash host passed its recorded before/after comparison on Android API 37. Its [results](splash/RESULTS.md) distinguish the expected baseline failure from the five passing candidate checks.
- The shared Expo app passed a four-ABI native source build and the recorded API 37 rendering/Activity smoke. See [its receipt](expo-app/validation.json).

The file manifest records the complete publishable tree except the manifest itself. It does not include private working directories or validation logs. No code from this package has been published.

Additional pinned Expo-monorepo checks completed after the ordinary Expo-app receipt:

- `et check-packages expo-modules-core expo-splash-screen`, scoped lint and format passed. Core: 277 tests / 43 suites / 8 snapshots; Splash: 16 tests / 8 suites. Both Jest tasks ran; eight build tasks used the local cache. See [package results](monorepo-checks/package-results.json).
- Both source-built `apps/bare-expo` variants completed three genuine JS reloads on API 37 with a stable process and new runtime/context markers. The Splash destruction/recreation owner probe passed.
- **The native state-prop probe did not pass.** Live updates failed in both variants. On keyed remounts, each value arrived in 2/4 candidate generations and 3/4 baseline generations. This small sample does not establish equivalence or rule out a regression. Both variants logged the same nonfatal reload and teardown warnings. See [full outcomes and fixture](monorepo-checks/README.md).

Open coverage limits: no API 31–33 smoke, no full-Expo-app heap/GC measurement and no additional native UNIT_TEST run. The source-level Fabric test and the native splash host each document their test doubles. No EAS or GitHub Actions build ran; these patches do not change config plugins. The historical `expo-app/validation.json` describes the earlier ordinary-app run, not the later monorepo checks.
