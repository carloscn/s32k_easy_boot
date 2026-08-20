# Standalone command-line build for the Easy_Boot (bootloader) project,
# independent of the S32DS IDE. Mirrors the compiler/linker flags S32DS
# itself uses (taken from Debug_FLASH/*.args after an IDE build), so the
# output matches what "Build Project" in S32DS produces.
#
# Usage:
#   make                 # build build/Easy_Boot.elf
#   make clean
#   make GCC_PATH=... RTD_BASE_PATH=...   # override toolchain/RTD location

GCC_PATH      ?= C:/NXP/S32DS.3.5/S32DS/build_tools/gcc_v10.2/gcc-10.2-arm32-eabi/bin
RTD_BASE_PATH ?= C:/NXP/S32DS.3.5/S32DS/software/PlatformSDK_S32K3/RTD

SRC_DIRS     = src src/hse RTD/src board generate/src Project_Settings/Startup_Code
PATH_BUILD   = build
PATH_OBJS    = build/objects

CC      = $(GCC_PATH)/arm-none-eabi-gcc
AS      = $(GCC_PATH)/arm-none-eabi-gcc -x assembler-with-cpp -g3
LD      = $(GCC_PATH)/arm-none-eabi-gcc
SIZE    = $(GCC_PATH)/arm-none-eabi-size

CFLAGS  = -std=c99 \
		  -DD_CACHE_ENABLE -DI_CACHE_ENABLE -DENABLE_FPU -DMPU_ENABLE -DGCC \
		  -DS32K3XX -DS32K312 -DCPU_S32K312 -DCPU_CORTEX_M7 \
		  -IRTD/include \
		  -Iinclude \
		  -Iinclude/public_inc \
		  -Iinclude/hse \
		  -Igenerate/include \
		  -Igenerate/src \
		  -Iboard \
		  -Iexternal/tja115x/include \
		  -Iexternal/UDS_Stack/TP/inc \
		  -Iexternal/UDS_Stack/TP/inc/CAN_TP \
		  -Iexternal/UDS_Stack/TP/inc/LIN_TP \
		  -Iexternal/UDS_Stack/UDS/inc \
		  -Iexternal/auto_lib/inc \
		  -Iexternal/flash_hal/inc \
		  -I"$(RTD_BASE_PATH)/BaseNXP_TS_T40D34M50I0R0/header" \
		  -I"$(RTD_BASE_PATH)/BaseNXP_TS_T40D34M50I0R0/include" \
		  -I"$(RTD_BASE_PATH)/Platform_TS_T40D34M50I0R0/include" \
		  -I"$(RTD_BASE_PATH)/Platform_TS_T40D34M50I0R0/startup/include" \
		  -Os -funsigned-char -fomit-frame-pointer -ggdb3 -pedantic -Wall -Wextra -c \
		  -fno-short-enums -funsigned-bitfields -fno-common -Wunused -Wstrict-prototypes \
		  -Wsign-compare -Werror=implicit-function-declaration -Wundef -Wdouble-promotion \
		  -mcpu=cortex-m7 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv5-sp-d16 \
		  -specs=nano.specs -specs=nosys.specs \
		  --sysroot="$(GCC_PATH)/../arm-none-eabi/lib"

LDFLAGS = -nostartfiles -Llib --entry=Reset_Handler -ggdb3 \
		  -T Project_Settings/Linker_Files/linker_flash_s32k312.ld \
		  -Wl,-Map,"$(PATH_BUILD)/Easy_Boot.map" \
		  -mcpu=cortex-m7 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv5-sp-d16 \
		  -specs=nano.specs -specs=nosys.specs \
		  --sysroot="$(GCC_PATH)/../arm-none-eabi/lib" \
		  -lc -lm -lgcc -l:libmbedcrypto.a

SRCS    = $(foreach d,$(SRC_DIRS),$(wildcard $(d)/*.c))
SRCS_AS = $(foreach d,$(SRC_DIRS),$(wildcard $(d)/*.s))
OBJS    = $(patsubst %.c,$(PATH_OBJS)/%.o,$(notdir $(SRCS))) $(patsubst %.s,$(PATH_OBJS)/%.o,$(notdir $(SRCS_AS)))

vpath %.c $(SRC_DIRS)
vpath %.s $(SRC_DIRS)

MKDIR = mkdir -p

.PHONY: all clean printsize build_timestamp

all: build_timestamp $(PATH_BUILD)/Easy_Boot.elf printsize

build_timestamp:
	python tools/gen_build_timestamp.py

$(PATH_BUILD) $(PATH_OBJS):
	$(MKDIR) $@

# The App is no longer embedded/linked in here - it's flashed to
# 0x00440000 completely independently via s32k_demo's own build+flash.
# easy_boot's boot.c reads app_metadata_t from a fixed runtime address
# regardless of how that flash region got programmed. See linker script
# comment and AGENTS.md for why this decoupling happened (a stale-embed
# bug: the old link-time embed had no Makefile dependency tracking, so App
# changes silently didn't take effect on reflash).
$(PATH_BUILD)/Easy_Boot.elf: $(OBJS) Project_Settings/Linker_Files/linker_flash_s32k312.ld | $(PATH_BUILD)
	$(LD) -o $@ $(OBJS) $(LDFLAGS)

$(PATH_OBJS)/%.o: %.c | $(PATH_OBJS)
	$(CC) $(CFLAGS) $< -o $@

$(PATH_OBJS)/%.o: %.s | $(PATH_OBJS)
	$(AS) $(CFLAGS) $< -o $@

# Not using -MMD -MP / auto dependency re-include here: on this toolchain,
# generated .d files escape Windows drive-letter colons as "C\:" which
# mingw32-make chokes on when re-including them on a second invocation
# (same issue documented in AGENTS.md / s32k_demo's Makefile).

printsize: $(PATH_BUILD)/Easy_Boot.elf
	$(SIZE) --format=berkeley $<

clean:
	rm -rf $(PATH_BUILD)
