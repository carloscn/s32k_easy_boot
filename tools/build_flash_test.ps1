# One-shot: build s32k_demo (app), build s32k_easy_boot (embeds the app's
# .bin at link time), flash the combined build/Easy_Boot.elf, capture UART.
#
# IMPORTANT KNOWN LIMITATION (see AGENTS.md): the debugger reset issued here
# only resets the ARM Cortex-M7 core ("M7resetcore" in S32K312.mac) - it does
# NOT re-run SBAF (the secure boot ROM). A debugger-reset test that shows
# little/no UART output does not prove the app is broken; only a physical
# power-cycle or reset-button press exercises the real SBAF -> easy_boot ->
# app boot chain. This script automates everything up to that point.
#
# Usage:
#   powershell -File tools/build_flash_test.ps1
#   powershell -File tools/build_flash_test.ps1 -SkipBuild        # flash+capture only
#   powershell -File tools/build_flash_test.ps1 -CaptureMs 20000

param(
    [switch]$SkipBuild,
    [int]$CaptureMs = 20000,
    [string]$Port = "COM3"
)

$ErrorActionPreference = "Stop"
$ToolsDir     = $PSScriptRoot
$EasyBootRoot = Split-Path $ToolsDir -Parent
$WorkspaceRoot = Split-Path $EasyBootRoot -Parent
$DemoRoot     = Join-Path $WorkspaceRoot "s32k_demo"
$ProvisionTools = Join-Path $WorkspaceRoot "s32k312_provision\tools"
$PegServer    = "C:\NXP\S32DS.3.5\eclipse\plugins\com.pemicro.debug.gdbjtag.pne_6.2.1.202606301718\win32\pegdbserver_console.exe"
$Gdb          = "C:\NXP\S32DS.3.5\eclipse\plugins\com.pemicro.debug.gdbjtag.pne_6.2.1.202606301718\win32\gdb\arm-none-eabi-gdb.exe"
$ElfPath      = Join-Path $EasyBootRoot "build\Easy_Boot.elf"
$GdbScript    = Join-Path $EasyBootRoot "build\flash_run.gdb"
$LogPath      = Join-Path $EasyBootRoot "build\uart_last.log"

function Invoke-Make($dir) {
    Write-Host "[build] mingw32-make -j4 in $dir"
    Push-Location $dir
    try {
        & mingw32-make -j4
        if ($LASTEXITCODE -ne 0) {
            # Retry once - a fresh/-rf'd objects dir under -j4 sometimes hits a
            # transient mkdir race on the first parallel invocation.
            Write-Host "[build] retrying once (possible parallel mkdir race)"
            & mingw32-make -j4
            if ($LASTEXITCODE -ne 0) { throw "make failed in $dir" }
        }
    } finally {
        Pop-Location
    }
}

if (-not $SkipBuild) {
    Invoke-Make $DemoRoot
    Invoke-Make $EasyBootRoot
}

if (-not (Test-Path $ElfPath)) {
    throw "Not found: $ElfPath (build failed or never ran)"
}
Write-Host "[flash] target: $ElfPath ($((Get-Item $ElfPath).LastWriteTime))"

@"
set pagination off
set confirm off
target remote localhost:7224
file $($ElfPath -replace '\\','/')
load
monitor reset
detach
quit
"@ | Set-Content -Path $GdbScript -Encoding ascii

Get-Process pegdbserver_console -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
Start-Sleep -Milliseconds 800
Start-Process $PegServer -ArgumentList "-startserver","-device=NXP_S32K3xx_S32K312","-serverport=7224" -WindowStyle Hidden
Start-Sleep -Seconds 2

$uartJob = Start-Job -ScriptBlock {
    param($ScriptPath, $Port, $DurationMs, $OutPath)
    & powershell -NoProfile -File $ScriptPath -Port $Port -Baud 115200 -DurationMs $DurationMs | Out-File -FilePath $OutPath -Encoding utf8
} -ArgumentList (Join-Path $ProvisionTools "uart_capture.ps1"), $Port, $CaptureMs, $LogPath

Start-Sleep -Milliseconds 500
& $Gdb -batch -x $GdbScript
Write-Host "[flash] done, waiting for UART capture ($CaptureMs ms)..."
Wait-Job $uartJob | Out-Null
Receive-Job $uartJob | Out-Null

Write-Host "`n===== UART capture ($LogPath) ====="
Get-Content $LogPath
Write-Host "===== end capture =====`n"
Write-Host "NOTE: if this is silent/short, that's expected for a debugger-only"
Write-Host "reset (core reset, no SBAF replay) - do a physical power-cycle for"
Write-Host "a real boot-chain test."
