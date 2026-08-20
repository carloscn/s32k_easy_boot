set pagination off
set confirm off
target remote localhost:7224
file c:/rom/01_Projects/work/s32k312_all/s32k_easy_boot/build/Easy_Boot.elf
load
monitor reset
detach
quit
