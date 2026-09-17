# STALE BUILD — do not flash this ELF for P3a

`build/Easy_Boot.elf` and `build/Easy_Boot.map` in this tree were last updated
in **`93c136c`** (2026-08-20). Protocol freeze sources landed in **`17df3cd`**
without a relink.

Gate before any board flash:

```bat
findstr boot_uds_app build\\Easy_Boot.map
findstr boot_img_prog build\\Easy_Boot.map
findstr boot_uart_prog build\\Easy_Boot.map
```

If missing → `mingw32-make clean && mingw32-make` (needs local `RTD/` /
`board/` / `generate/`), then flash **Boot only**. Never gdb-load App ELF over
Boot (PEMicro may mass-erase `0x00400000`).

On-board pass (mock does not count): SW6 → `10 02` → poll `22 F180` →
`22 A0F1` (13 B, max `0xCA000`) → small `FF00` / `34` / `36` / `37` / `0202`.
Skip `0203` (`commitImplemented=false`).
