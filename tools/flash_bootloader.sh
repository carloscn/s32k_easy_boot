#!/usr/bin/env bash
# Build + flash ONLY s32k_easy_boot (bootloader), completely independent of
# the App. The App's flash region (0x00440000+) is never touched by this -
# easy_boot no longer embeds the App at link time (see linker script /
# AGENTS.md for why that changed).
#
# Usage:
#   bash tools/flash_bootloader.sh                 # build + flash
#   bash tools/flash_bootloader.sh --skip-build     # flash only

set -euo pipefail

TOOLS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EASYBOOT_ROOT="$(cd "$TOOLS_DIR/.." && pwd)"
PEG_SERVER="C:\\NXP\\S32DS.3.5\\eclipse\\plugins\\com.pemicro.debug.gdbjtag.pne_6.2.1.202606301718\\win32\\pegdbserver_console.exe"
GDB="C:\\NXP\\S32DS.3.5\\eclipse\\plugins\\com.pemicro.debug.gdbjtag.pne_6.2.1.202606301718\\win32\\gdb\\arm-none-eabi-gdb.exe"
ELF_PATH="$EASYBOOT_ROOT/build/Easy_Boot.elf"
GDB_SCRIPT="$EASYBOOT_ROOT/build/flash_bootloader.gdb"
SKIP_BUILD=0

[ "${1:-}" = "--skip-build" ] && SKIP_BUILD=1

if [ "$SKIP_BUILD" -eq 0 ]; then
    echo "[build] mingw32-make -j4 in $EASYBOOT_ROOT"
    ( cd "$EASYBOOT_ROOT" && mingw32-make -j4 ) || ( cd "$EASYBOOT_ROOT" && mingw32-make -j4 )
fi

if [ ! -f "$ELF_PATH" ]; then
    echo "Not found: $ELF_PATH" >&2
    exit 1
fi
echo "[flash] target: $ELF_PATH ($(date -r "$ELF_PATH"))"

ELF_PATH_WIN="$(echo "$ELF_PATH" | sed -E 's#^/([a-zA-Z])/#\1:/#')"
cat > "$GDB_SCRIPT" <<EOF
set pagination off
set confirm off
target remote localhost:7224
file ${ELF_PATH_WIN}
load
monitor reset
detach
quit
EOF

powershell -NoProfile -Command "
Get-Process pegdbserver_console -EA SilentlyContinue | Stop-Process -Force -EA SilentlyContinue
Start-Sleep -Milliseconds 800
Start-Process '$PEG_SERVER' -ArgumentList '-startserver','-device=NXP_S32K3xx_S32K312','-serverport=7224' -WindowStyle Hidden
Start-Sleep -Seconds 2
"

"$GDB" -batch -x "$GDB_SCRIPT"
echo "[flash] bootloader flash done."
