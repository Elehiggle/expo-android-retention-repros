#!/usr/bin/env python3
"""Select reviewed Expo core source without silently overwriting a different patch."""
import hashlib,json,pathlib,sys
root=pathlib.Path(__file__).resolve().parent
mode=sys.argv[1] if len(sys.argv)==2 else None
if mode not in ('before','after'):raise SystemExit('Usage: python3 select-core-source.py before|after')
source=root/'core-source';manifest=json.loads((source/'manifest.json').read_text())
package=root/'node_modules'/manifest['package']
if not (package/'package.json').exists():raise SystemExit('Install the pinned local npm dependencies first.')
info=json.loads((package/'package.json').read_text())
if info['version']!=manifest['version']:raise SystemExit('Unexpected expo-modules-core version: '+info['version'])
target=package/manifest['packagePath']
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest() if p.exists() else None
for name,row in manifest['files'].items():
 if digest(target/name) not in (row['before'],row['after']):raise SystemExit('Unexpected current source: '+name)
 expected=row[mode]
 if digest(source/mode/name)!=expected:raise SystemExit('Repro source checksum mismatch: '+name)
for name,row in manifest['files'].items():
 path=target/name
 if row[mode] is None:
  if path.exists():path.unlink()
 else:path.write_bytes((source/mode/name).read_bytes())
 for_check=digest(path)
 if for_check!=row[mode]:raise SystemExit('Post-write verification failed: '+name)
print('expo-modules-core '+manifest['version']+': selected '+mode+'; verified '+str(len(manifest['files']))+' native source files')
