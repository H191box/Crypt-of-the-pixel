/**
 * palettes.h — Color Palette Definitions
 * 
 * GBA uses 15-bit RGB color (5 bits per channel: 0-31).
 * Format: RRRRR GGGGG BBBBB (15-bit packed into u16).
 * 
 * We define:
 * - Dungeon palette (walls, floors, environment)
 * - Character palette (player, enemies)
 * - UI palette (health bars, text, indicators)
 */

#ifndef PALETTES_H
#define PALETTES_H

#include "gba_types.h"

/* ---- Color helper macro ---- */
#define RGB(r, g, b)  ((u16)(((r) & 0x1F) | (((g) & 0x1F) << 5) | (((b) & 0x1F) << 10)))

/* ---- Palette sizes ---- */
#define PAL_DUNGEON_SIZE     32   /* 32 colors for dungeon tiles */
#define PAL_CHARACTER_SIZE   32   /* 32 colors for characters */
#define PAL_UI_SIZE          16   /* 16 colors for UI elements */

/* ---- Dungeon tile palette (background, bank 0) ---- */
/* Stone grays, earth browns, water blues, mossy greens */
extern const u16 pal_dungeon[PAL_DUNGEON_SIZE];

/* ---- Character/sprite palette (sprite, bank 0) ---- */
/* Skin tones, armor, clothing */
extern const u16 pal_character[PAL_CHARACTER_SIZE];

/* ---- UI palette (text, bars, indicators) ---- */
extern const u16 pal_ui[PAL_UI_SIZE];

/* ---- Font palette (reuses UI colors) ---- */
extern const u16 font_palette[16];

/* ---- Load all palettes into VRAM ---- */
void load_all_palettes(void);

#endif /* PALETTES_H */
