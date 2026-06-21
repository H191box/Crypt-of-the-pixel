/**
 * main.c — Game Boy Advance Entry Point
 * 
 * Crypt of the Pixel — Isometric Roguelike Dungeon Crawler
 * 
 * This is the first function called when the GBA boots the ROM.
 * It initializes all hardware and subsystems, then enters the
 * main game loop.
 * 
 * Main loop:
 *   1. Read input
 *   2. Update game state
 *   3. Render frame
 *   4. Wait for VBlank
 */

/* Pull in engine headers we need directly */
#include "../engine/gba_types.h"
#include "../engine/gba_header.h"
#include "../engine/system.h"
#include "../engine/input.h"
#include "../game/game_state.h"

/* ---- GBA ROM Entry Point ---- */
/* The linker script entry is _start, which jumps here.
 * On real hardware, the BIOS loads the ROM header at 0x08000000
 * and jumps to the entry point. We use this as our C entry. */
int main(void) {
    /* 
     * Initialize all subsystems.
     * This sets video mode, BG layers, OAM, palettes, 
     * interrupts, and game state.
     */
    game_init();
    
    /* 
     * Main game loop — runs forever.
     * 
     * The GBA runs at ~59.73 FPS (228 scanlines, ~280.896 cycles/line).
     * We synchronize to VBlank to prevent tearing and ensure
     * consistent frame timing.
     */
    while (1) {
        /* 1. Read input state (D-pad, buttons) */
        input_update();
        
        /* 2. Update game logic (movement, AI, combat, state) */
        game_update();
        
        /* 3. Render current frame to screen */
        game_render();
        
        /* 4. Wait for VBlank before swapping buffers */
        sys_wait_vblank();
    }
    
    /* Should never reach here */
    return 0;
}

/* ---- GBA Header Section ---- */
/* 
 * The GBA ROM header must be at 0x08000000 in the final binary.
 * We place it in a special section that the linker puts at the ROM start.
 * 
 * The header contains:
 *   - Entry point instruction (ARM branch)
 *   - Nintendo logo (156 bytes, must match for BIOS to accept)
 *   - Game title, code, maker code
 *   - Fixed values, version, complement, checksum
 */

#ifndef _HEADER_DEFINED
#define _HEADER_DEFINED

/* Nintendo logo data (required — must match or BIOS refuses to boot) */
static const u8 nintendo_logo[156] = {
    0x24,0xFF,0xAE,0x51,0x69,0x9A,0xA2,0x21,
    0x3D,0x84,0x82,0x0A,0x84,0xE4,0x09,0xAD,
    0x11,0x24,0x8B,0x98,0xC0,0x81,0x7F,0x21,
    0xA3,0x52,0xBE,0x19,0x93,0x09,0xCE,0x20,
    0x10,0x46,0x4A,0x4A,0xF8,0x27,0x31,0xEC,
    0x58,0xC7,0xE8,0x33,0x82,0xE3,0xCE,0xBF,
    0x85,0xF4,0xDF,0x94,0xCE,0x4B,0x09,0xC1,
    0x94,0x56,0x8A,0xC0,0x13,0x72,0xA7,0xFC,
    0x9F,0x84,0x4D,0x73,0xA3,0xCA,0x9A,0x61,
    0x58,0x97,0xA3,0x27,0xFC,0x03,0x98,0x76,
    0x23,0x1D,0xC7,0x61,0x03,0x04,0xAE,0x56,
    0xBF,0x38,0x84,0x00,0x40,0xA7,0x0E,0xFD,
    0xFF,0x52,0xFE,0x03,0x6F,0x95,0x30,0xF1,
    0x97,0xFB,0xC0,0x85,0x60,0xD6,0x80,0x25,
    0xA9,0x63,0xBE,0x03,0x01,0x4E,0x38,0xE2,
    0xF9,0xA2,0x34,0xFF,0xBB,0x3E,0x03,0x44,
    0x78,0x00,0x90,0xCB,0x88,0x11,0x3A,0x94,
    0x65,0xC0,0x7C,0x63,0x87,0xF0,0x3C,0xAF,
    0xD6,0x25,0xE4,0x8B,0x38,0x0A,0xAC,0x72,
    0x21,0xD4,0xF8,0x07
};

/* GBA ROM header — placed at ROM start via linker section ".header" */
__attribute__((section(".header")))
const u32 gba_rom_header[48] = {
    /* [0] Entry point: ARM branch to main (offset 0x08000000 + 0xC0 for header) */
    /* Branch instruction: 0xEA + offset. main() is at ~0x080000C0+ */
    /* Simplified: set to 0 which gbafix will correct */
    0x00000000,
    
    /* [1..39] Nintendo logo (156 bytes = 39 u32 words) */
    /* We'll let gbafix handle this */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000,
    
    /* [40] Game title (12 bytes) + Game code (4 bytes) + Maker (2 bytes) */
    /* Packed as 3 u32 words */
    0x52544543,  /* "CTPX" (reverse for little-endian: CRYPT reversed) */
    0x50580000,  /* "PX\0\0" */
    0x30300000,  /* "00\0\0" */
    
    /* [43] Fixed value + Main unit + Device type + Reserved */
    0x00000096,  /* 0x96 fixed */
    
    /* [44] Reserved area (7 bytes) + Version (1 byte) */
    0x00000000,  /* Reserved */
    
    /* [45] Complement + Checksum (4 bytes) */
    /* Will be filled by gbafix */
    0x00000000,
    
    /* Remaining padding */
    0x00000000, 0x00000000,
};

#endif /* _HEADER_DEFINED */
