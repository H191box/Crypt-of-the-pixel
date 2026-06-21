# =============================================================================
# Crypt of the Pixel — Isometric Roguelike Dungeon Crawler for GBA
# Makefile for devkitARM toolchain
# =============================================================================

# ---- Toolchain ----
AS      = arm-none-eabi-as
CC      = arm-none-eabi-gcc
CXX     = arm-none-eabi-g++
LD      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

# ---- devkitARM paths (override if needed) ----
LIBGBA  ?= $(DEVKITPRO)/libgba
LIBTONC ?= $(DEVKITPRO)/libtonc

# ---- Project dirs ----
BUILD   = build
SRC     = src
INCLUDE = include

# ---- Source files ----
CFILES  := $(foreach dir,$(SRC),$(wildcard $(dir)/*.c))
SFILES  := $(foreach dir,$(SRC),$(wildcard $(dir)/*.s))

OFILES  := $(patsubst %.c,$(BUILD)/%.o,$(CFILES))
OFILES  += $(patsubst %.s,$(BUILD)/%.o,$(SFILES))

# ---- Output ----
TARGET  = crypt_of_the_pixel
ELF     = $(BUILD)/$(TARGET).elf
GBA     = $(BUILD)/$(TARGET).gba
MAP     = $(BUILD)/$(TARGET).map

# ---- Compiler flags ----
ARCH    = -mthumb-interwork -mthumb

CFLAGS  = $(ARCH) -O2 -Wall -Wextra -Wno-unused-parameter
CFLAGS += -I$(INCLUDE) -I$(SRC)
CFLAGS += -ffreestanding -fno-builtin -fno-strict-aliasing
CFLAGS += -fno-common -mlong-calls
CFLAGS += -DGBA

ASFLAGS = $(ARCH)

LDFLAGS = $(ARCH) -Wl,-Map,$(MAP) -Wl,--gc-sections
LDFLAGS += -T gba.ld -nostartfiles -nostdlib

# ---- Default target ----
all: $(GBA)

# ---- Link ELF ----
$(ELF): $(OFILES)
	@echo " Linking $@..."
	$(LD) $(LDFLAGS) -o $@ $(OFILES)

# ---- Build GBA ROM ----
$(GBA): $(ELF)
	@echo " Building $@..."
	$(OBJCOPY) -O binary $< $@
	@echo " Fixing header..."
	-@gbafix $@ -t"CryptPixel" -c"CTPX" -m00 -r00 2>/dev/null || true
	@echo " Done! ROM: $@"
	@ls -la $@

# ---- Compile C ----
$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo " CC $<"
	$(CC) $(CFLAGS) -c -o $@ $<

# ---- Compile ASM ----
$(BUILD)/%.o: %.s
	@mkdir -p $(dir $@)
	@echo " AS $<"
	$(AS) $(ASFLAGS) -o $@ $<

# ---- GBA linker script (embedded) ----
gba.ld:
	@echo "Creating linker script..."
	@echo 'OUTPUT_ARCH(arm)' > gba.ld
	@echo 'ENTRY(_start)' >> gba.ld
	@echo 'MEMORY {' >> gba.ld
	@echo '  rom   : ORIGIN = 0x08000000, LENGTH = 32M' >> gba.ld
	@echo '  ewram : ORIGIN = 0x02000000, LENGTH = 256K' >> gba.ld
	@echo '  iwram : ORIGIN = 0x03000000, LENGTH = 32K' >> gba.ld
	@echo '}' >> gba.ld
	@echo 'SECTIONS {' >> gba.ld
	@echo '  .rom  0x08000000 : { *(.header) *(.text) *(.rodata) } > rom' >> gba.ld
	@echo '  .data : { *(.data) } > ewram AT> rom' >> gba.ld
	@echo '  .bss  : { *(.bss) *(COMMON) } > ewram' >> gba.ld
	@echo '  .iwram : { *(.iwram) *(.iwram.*) } > iwram' >> gba.ld
	@echo '  /DISCARD/ : { *(.ARM.attributes) }' >> gba.ld
	@echo '}' >> gba.ld

# ---- Linker script is a prerequisite ----
$(OFILES): gba.ld

# ---- Clean ----
clean:
	rm -rf $(BUILD)/*.o $(BUILD)/*.elf $(BUILD)/*.gba $(BUILD)/*.map

.PHONY: all clean
