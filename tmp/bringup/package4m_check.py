from pathlib import Path
import shutil
import subprocess
import re
import os

root = Path(__file__).resolve().parents[2]
tools = root / 'SDK/cpu/br35/tools'
out = root / 'tmp/bringup/flash4m-package'
out.mkdir(parents=True, exist_ok=True)
ini = (tools / 'isd_config.ini').read_text(encoding='utf-8', errors='replace')
assert not re.search(r'^\s*(MODE_FILE|FATFSI_FILE)\s*=', ini, re.M)
assert re.search(r'^\s*DATA_ADR\s*=\s*0x1D6000', ini, re.M)
assert re.search(r'^\s*DATA_LEN\s*=\s*0x28000', ini, re.M)
batch = (tools / 'download.bat').read_text(encoding='utf-8', errors='replace')
assert 'call download/watch/download.bat' in batch
for name in ['isd_config.ini', 'uboot.boot', 'ota.bin', 'cfg_tool.bin',
             'p11_code.bin', 'stream.bin', 'flash_params_v3.bin',
             'br35loader.bin', 'script.ver']:
    shutil.copy2(tools / name, out / name)
config = tools / 'download/watch/config.dat'
if config.exists():
    shutil.copy2(config, out / 'config.dat')
objcopy = Path('C:/JL/pi32/bin/llvm-objcopy.exe')
elf = tools / 'sdk.elf'
sections = ['text', 'data', 'data_code', 'overlay_aec', 'overlay_aac',
            'ps_ram_data_code', 'dcache_ram_data', 'icache_ram_data_code']
binary = bytearray()
for section in sections:
    dest = out / f'{section}.bin'
    res_obj = subprocess.run([str(objcopy), '-O', 'binary', '-j', '.' + section,
                    'sdk.elf', os.path.relpath(dest, tools)], cwd=tools,
                    stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if res_obj.returncode:
        raise RuntimeError(f'{section}: {res_obj.stdout.decode(errors="replace")}')
    binary.extend(dest.read_bytes())
(out / 'app.bin').write_bytes(binary)
assert len(binary) < 0x1D6000, f'app.bin {len(binary)} overlaps DATA at 0x1D6000'
assert 0x1D6000 + 0x28000 <= 0x400000
print(f'app.bin = {len(binary)} bytes; DATA end = 0x1FE000; flash end = 0x400000')
print('Static capacity audit only; this script does not produce a burnable firmware file.')
