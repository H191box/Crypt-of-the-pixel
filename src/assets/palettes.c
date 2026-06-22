/**
 * palettes.c — Palette Data
 * 
 * 15-bit RGB: RGB(r,g,b) macro packs 5 bits per channel.
 * Each palette bank has 16 colors (indices 0-15).
 * Color 0 is transparent for sprites.
 */

#include "gba_types.h"
#include "gba_header.h"
#include "palettes.h"
#include "dma.h"

/* ---- Dungeon tile palette ---- */
const u16 pal_dungeon[PAL_DUNGEON_SIZE] = {
    /* 0 */  RGB(0,  0,  0),   /* Black (transparent for void) */
    /* 1 */  RGB(24, 24, 26),  /* Dark stone */
    /* 2 */  RGB(18, 18, 20),  /* Darker stone (wall shadow) */
    /* 3 */  RGB(28, 28, 30),  /* Medium stone */
    /* 4 */  RGB(20, 20, 22),  /* Stone crack line */
    /* 5 */  RGB(16, 16, 18),  /* Dark mortar */
    /* 6 */  RGB(30, 28, 25),  /* Warm stone (alternate floor) */
    /* 7 */  RGB(14, 14, 16),  /* Very dark stone */
    /* 8 */  RGB(22, 22, 24),  /* Stone highlight */
    /* 9 */  RGB(12, 14, 12),  /* Moss green */
    /* 10 */ RGB(6,  10, 20),  /* Deep water */
    /* 11 */ RGB(10, 16, 28),  /* Water */
    /* 12 */ RGB(14, 20, 31),  /* Water highlight */
    /* 13 */ RGB(26, 20, 12),  /* Wood/door */
    /* 14 */ RGB(20, 15,  8),  /* Dark wood */
    /* 15 */ RGB(30, 24, 16),  /* Light wood */
    /* 16 */ RGB(8,  6,  4),   /* Stairs dark */
    /* 17 */ RGB(20, 16, 10),  /* Stairs mid */
    /* 18 */ RGB(28, 22, 14),  /* Stairs light */
    /* 19 */ RGB(31, 26, 8),   /* Gold/treasure */
    /* 20 */ RGB(26, 20, 4),   /* Gold dark */
    /* 21 */ RGB(10, 8,  6),   /* Treasure chest dark */
    /* 22 */ RGB(18, 12, 6),   /* Treasure chest mid */
    /* 23 */ RGB(14, 6,  4),   /* Trap/red accent */
    /* 24 */ RGB(31, 24, 10),  /* Torch flame */
    /* 25 */ RGB(26, 18, 4),   /* Torch mid */
    /* 26 */ RGB(18, 12, 4),   /* Torch base */
    /* 27 */ RGB(24, 20, 28),  /* Altar stone */
    /* 28 */ RGB(18, 14, 22),  /* Altar dark */
    /* 29 */ RGB(16, 12, 8),   /* Bone white */
    /* 30 */ RGB(20, 18, 14),  /* Bone yellow */
    /* 31 */ RGB(10, 10, 10),  /* Shadow/ambient dark */
};

/* ---- Character palette (for sprites) ---- */
const u16 pal_character[PAL_CHARACTER_SIZE] = {
    /* 0 */  RGB(0,  0,  0),   /* Transparent */
    /* 1 */  RGB(31, 26, 22), /* Skin tone light */
    /* 2 */  RGB(26, 20, 16), /* Skin tone mid */
    /* 3 */  RGB(20, 14, 10), /* Skin tone dark */
    /* 4 */  RGB(24, 20, 16), /* Hair/skin shadow */
    /* 5 */  RGB(12, 10, 16), /* Armor dark (plate) */
    /* 6 */  RGB(18, 16, 22), /* Armor mid */
    /* 7 */  RGB(24, 22, 28), /* Armor light */
    /* 8 */  RGB(8,  6,  12), /* Armor outline */
    /* 9 */  RGB(20, 16, 12), /* Leather (thief) */
    /* 10 */ RGB(14, 10, 6),  /* Leather dark */
    /* 11 */ RGB(26, 22, 18), /* Leather light */
    /* 12 */ RGB(16, 8,  24), /* Robe (mage) */
    /* 13 */ RGB(22, 14, 28), /* Robe mid */
    /* 14 */ RGB(28, 20, 31), /* Robe light */
    /* 15 */ RGB(10, 4,  16), /* Robe dark */
    /* 16 */ RGB(10, 28, 10), /* Slime green */
    /* 17 */ RGB(6,  22, 6),  /* Slime mid */
    /* 18 */ RGB(16, 31, 16), /* Slime highlight */
    /* 19 */ RGB(4,  16, 4),  /* Slime dark */
    /* 20 */ RGB(24, 24, 20), /* Bone/skeleton */
    /* 21 */ RGB(18, 18, 14), /* Bone mid */
    /* 22 */ RGB(30, 30, 26), /* Bone light */
    /* 23 */ RGB(8,  8,  6),  /* Bone outline */
    /* 24 */ RGB(24, 22, 28), /* Ghost body */
    /* 25 */ RGB(18, 16, 22), /* Ghost mid */
    /* 26 */ RGB(28, 28, 30), /* Ghost highlight */
    /* 27 */ RGB(20, 18, 12), /* Bat wing */
    /* 28 */ RGB(14, 12, 8),  /* Bat dark */
    /* 29 */ RGB(26, 24, 20), /* Bat body */
    /* 30 */ RGB(20, 4,  4),  /* Red enemy accent */
    /* 31 */ RGB(31, 8,  8),  /* Bright red (boss) */
};

/* ---- UI/Font palette ---- */
const u16 pal_ui[PAL_UI_SIZE] = {
    /* 0 */  RGB(0,  0,  0),   /* Black */
    /* 1 */  RGB(31, 31, 31),  /* White */
    /* 2 */  RGB(20, 20, 22),  /* Gray */
    /* 3 */  RGB(31, 8,  8),   /* Red */
    /* 4 */  RGB(8,  24, 8),   /* Green */
    /* 5 */  RGB(8,  12, 28),  /* Blue */
    /* 6 */  RGB(31, 28, 8),   /* Yellow */
    /* 7 */  RGB(31, 16, 4),   /* Orange */
    /* 8 */  RGB(12, 10, 10),  /* Dark gray */
    /* 9 */  RGB(24, 22, 22),  /* Light gray */
    /* 10 */ RGB(16, 4,  4),   /* Dark red */
    /* 11 */ RGB(4,  16, 4),   /* Dark green */
    /* 12 */ RGB(4,  8,  20),  /* Dark blue */
    /* 13 */ RGB(20, 18, 4),   /* Dark yellow */
    /* 14 */ RGB(24, 12, 8),   /* Brown */
    /* 15 */ RGB(8,  8,  8),   /* Near black */
};

/* ---- Font palette (same as UI) ---- */
const u16 font_palette[16] = {
    RGB(0,  0,  0),    /* 0: Black background */
    RGB(31, 31, 31),   /* 1: White text */
    RGB(20, 20, 22),   /* 2: Gray */
    RGB(31, 8,  8),     /* 3: Red */
    RGB(8,  24, 8),     /* 4: Green */
    RGB(8,  12, 28),    /* 5: Blue */
    RGB(31, 28, 8),     /* 6: Yellow */
    RGB(31, 16, 4),     /* 7: Orange */
    RGB(12, 10, 10),    /* 8: Dark gray */
    RGB(24, 22, 22),    /* 9: Light gray */
    RGB(16, 4,  4),     /* 10: Dark red */
    RGB(4,  16, 4),     /* 11: Dark green */
    RGB(4,  8,  20),    /* 12: Dark blue */
    RGB(20, 18, 4),     /* 13: Dark yellow */
    RGB(24, 12, 8),     /* 14: Brown */
    RGB(8,  8,  8),     /* 15: Near black */
};

/**
 * Load all palettes into VRAM.
 */
void load_all_palettes(void) {
    /* Background palette: dungeon tiles in bank 0 (colors 0-31) */
    dma_to_palette(pal_dungeon, PAL_DUNGEON_SIZE, TRUE);
    
    /* Sprite palette: character/enemy colors in sprite palette bank 0 */
    dma_to_palette(pal_character, PAL_CHARACTER_SIZE, FALSE);
    
    /* UI palette in background bank 1 (offset 16-31) */
    for (int i = 0; i < PAL_UI_SIZE; i++) {
        PAL_BG[16 + i] = pal_ui[i];
    }
    
    /* ---- CRITICAL FIX: Copy font palette to ALL 16 palette banks ----
     * The text renderer uses the TEXT_COLOR_x values (1..9) as palette
     * bank numbers. If a bank is all-black, text in that color is
     * invisible (black fg on black bg). We duplicate font_palette
     * into every bank so any color index works.
     * Bank 0 already has dungeon data (darker), but for text-only BG3
     * the font palette is what matters. We override all 16 banks. */
    for (int bank = 0; bank < 16; bank++) {
        for (int c = 0; c < 16; c++) {
            PAL_BG[bank * 16 + c] = font_palette[c];
        }
    }
}
