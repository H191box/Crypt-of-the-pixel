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
CFILES  := $(shell find $(SRC) -name '*.c')
SFILES  := $(shell find $(SRC) -name '*.s')

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
CFLAGS += -I$(INCLUDE) $(shell find $(SRC) -type d -exec echo -I{} \;)
CFLAGS += -ffreestanding -fno-builtin -fno-strict-aliasing
CFLAGS += -fno-common -mlong-calls
CFLAGS += -DGBA

ASFLAGS = -mthumb-interwork

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

# ---- Clean ----
clean:
	rm -rf build

.PHONY: all clean
