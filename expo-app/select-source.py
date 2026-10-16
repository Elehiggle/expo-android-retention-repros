from pathlib import Path
import hashlib, json, sys
root = Path(__file__).resolve().parent
variant = sys.argv[1] if len(sys.argv) == 2 else ""
if variant not in ("before", "after"):
    raise SystemExit("usage: python3 select-source.py before|after")
pkg = root / "node_modules/expo-splash-screen"
if json.loads((pkg / "package.json").read_text())["version"] != "58.0.9":
    raise SystemExit("Install expo-splash-screen@58.0.9 before applying this reproduction's source")
destination = pkg / "android/src/main/java/expo/modules/splashscreen/SplashScreenManager.kt"
allowed = [(root / "sources" / name / "SplashScreenManager.kt").read_bytes() for name in ("before", "after")]
if destination.read_bytes() not in allowed:
    raise SystemExit("Installed manager differs from both pinned sources; refusing to overwrite it")
source = (root / "sources" / variant / "SplashScreenManager.kt").read_bytes()
destination.write_bytes(source)
print(variant, hashlib.sha256(source).hexdigest())
