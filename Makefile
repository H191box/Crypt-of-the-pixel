# =============================================================================
# Crypt of the Pixel — GBA Makefile
# =============================================================================

# ---- Toolchain ----
TOOLCHAIN = /tmp/arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi
AS      = $(TOOLCHAIN)/bin/arm-none-eabi-as
CC      = $(TOOLCHAIN)/bin/arm-none-eabi-gcc
LD      = $(TOOLCHAIN)/bin/arm-none-eabi-gcc
OBJCOPY = $(TOOLCHAIN)/bin/arm-none-eabi-objcopy
OBJDUMP = $(TOOLCHAIN)/bin/arm-none-eabi-objdump

# ---- Project dirs ----
BUILD   = build
SRC     = src
INCLUDE = include

# ---- Source files ----
CFILES  := $(shell find $(SRC) -name '*.c')
SFILES  := $(shell find $(SRC) -name '*.s')

# ---- Object files: crt0.o MUST be first! ----
OFILES  := $(BUILD)/src/main/crt0.o
OFILES  += $(patsubst %.c,$(BUILD)/%.o,$(CFILES))
OFILES  += $(filter-out $(BUILD)/src/main/crt0.o,$(patsubst %.s,$(BUILD)/%.o,$(SFILES)))

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

LDFLAGS = $(ARCH) -Wl,-Map,$(MAP)
LDFLAGS += -T gba.ld -nostartfiles -nostdlib

# ---- Default target ----
all: $(GBA)

# ---- Link ELF ----
$(ELF): $(OFILES)
	@echo " Linking $@..."
	$(LD) $(LDFLAGS) -o $@ $(OFILES)

# ---- Build GBA ROM (only .text + .rodata + .data sections) ----
$(GBA): $(ELF)
	@echo " Building ROM..."
	$(OBJCOPY) -O binary -j .text -j .rodata -j .data $< $@
	@echo " ROM size:"
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

# ---- Debug helpers ----
hexdump: $(GBA)
	xxd $(GBA) | head -20

disasm: $(ELF)
	$(OBJDUMP) -d $(ELF) | head -100

.PHONY: all clean hexdump disasm
