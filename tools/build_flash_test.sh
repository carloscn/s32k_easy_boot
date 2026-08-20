#!/usr/bin/env bash
# One-shot: build s32k_demo (app), build s32k_easy_boot (embeds the app's
# .bin at link time), flash the combined build/Easy_Boot.elf, capture UART.
#
# IMPORTANT KNOWN LIMITATION (see AGENTS.md "NXP mbedTLS-with-HSE port"
# section): the debugger reset issued here only resets the ARM Cortex-M7
# core ("M7resetcore" in S32K312.mac, confirmed by inspecting that file) - it
# does NOT re-run SBAF (the secure boot ROM). A debugger-reset test that
# shows little/no UART output does not prove the app is broken; only a
# physical power-cycle or reset-button press exercises the real
# SBAF -> easy_boot -> app boot chain. This script automates the build,
# flash, and capture - the physical reset is the one step it cannot do.
#
# Usage:
#   bash tools/build_flash_test.sh                  # build + flash + capture
#   bash tools/build_flash_test.sh --skip-build      # flash + capture only
#   bash tools/build_flash_test.sh --capture-ms 30000

set -euo pipefail

TOOLS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EASYBOOT_ROOT="$(cd "$TOOLS_DIR/.." && pwd)"
WORKSPACE_ROOT="$(cd "$EASYBOOT_ROOT/.." && pwd)"
DEMO_ROOT="$WORKSPACE_ROOT/s32k_demo"
PROVISION_TOOLS="$WORKSPACE_ROOT/s32k312_provision/tools"
PEG_SERVER="C:\\NXP\\S32DS.3.5\\eclipse\\plugins\\com.pemicro.debug.gdbjtag.pne_6.2.1.202606301718\\win32\\pegdbserver_console.exe"
GDB="C:\\NXP\\S32DS.3.5\\eclipse\\plugins\\com.pemicro.debug.gdbjtag.pne_6.2.1.202606301718\\win32\\gdb\\arm-none-eabi-gdb.exe"
ELF_PATH="$EASYBOOT_ROOT/build/Easy_Boot.elf"
GDB_SCRIPT="$EASYBOOT_ROOT/build/flash_run.gdb"
LOG_PATH="$EASYBOOT_ROOT/build/uart_last.log"
PORT="COM3"
CAPTURE_MS=20000
SKIP_BUILD=0

while [ $# -gt 0 ]; do
    case "$1" in
        --skip-build) SKIP_BUILD=1; shift ;;
        --capture-ms) CAPTURE_MS="$2"; shift 2 ;;
        --port) PORT="$2"; shift 2 ;;
        *) echo "unknown arg: $1" >&2; exit 1 ;;
    esac
done

make_with_retry() {
    local dir="$1"
    echo "[build] mingw32-make -j4 in $dir"
    ( cd "$dir" && mingw32-make -j4 ) || {
        echo "[build] retrying once (possible parallel mkdir race on first -j4 run)"
        ( cd "$dir" && mingw32-make -j4 )
    }
}

if [ "$SKIP_BUILD" -eq 0 ]; then
    make_with_retry "$DEMO_ROOT"
    make_with_retry "$EASYBOOT_ROOT"
fi

if [ ! -f "$ELF_PATH" ]; then
    echo "Not found: $ELF_PATH (build failed or never ran)" >&2
    exit 1
fi
echo "[flash] target: $ELF_PATH ($(date -r "$ELF_PATH"))"

mkdir -p "$(dirname "$GDB_SCRIPT")"
# gdb is a native Windows exe and needs a Windows-style path (C:/...), not
# bash's /c/... mount-style path.
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

UART_SCRIPT_WIN="$(echo "$PROVISION_TOOLS/uart_capture.ps1" | sed -E 's#^/([a-zA-Z])/#\1:/#')"
powershell -NoProfile -File "$UART_SCRIPT_WIN" -Port "$PORT" -Baud 115200 -DurationMs "$CAPTURE_MS" > "$LOG_PATH" &
CAPTURE_PID=$!

sleep 1
"$GDB" -batch -x "$GDB_SCRIPT"

echo "[flash] done, waiting for UART capture (${CAPTURE_MS}ms)..."
wait "$CAPTURE_PID"

echo ""
echo "===== UART capture ($LOG_PATH) ====="
cat "$LOG_PATH"
echo "===== end capture ====="
echo ""
echo "NOTE: if this is silent/short, that's expected for a debugger-only"
echo "reset (core reset, no SBAF replay) - do a physical power-cycle for"
echo "a real boot-chain test."
