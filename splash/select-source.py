#!/usr/bin/env python3
from pathlib import Path
import sys, shutil, hashlib
root = Path(__file__).resolve().parent
variant = sys.argv[1] if len(sys.argv) == 2 else ""
if variant not in ("before", "after"):
    raise SystemExit("usage: python3 select-source.py before|after")
source = root / "sources" / variant / "SplashScreenManager.kt"
dest = root / "app/src/main/java/expo/modules/splashscreen/SplashScreenManager.kt"
shutil.copyfile(source, dest)
print(variant, hashlib.sha256(dest.read_bytes()).hexdigest())
