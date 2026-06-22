/**
 * text_renderer.h — Simple 8×8 Bitmap Font Text Renderer
 * 
 * Renders text to VRAM for the HUD layer (BG3).
 * Uses a built-in 1bpp font stored as const data.
 * Draws directly to the BG tilemap — each character occupies
 * one 8×8 GBA tile.
 * 
 * Supports:
 * - Basic ASCII printable characters (32-126)
 * - Integer and number rendering
 * - Color selection via palette index
 */

#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include "gba_types.h"

/* ---- Text rendering configuration ---- */
#define FONT_CHAR_WIDTH    8
#define FONT_CHAR_HEIGHT   8
#define FONT_FIRST_CHAR    32   /* ASCII space */
#define FONT_LAST_CHAR     126  /* ASCII ~ */
#define FONT_CHAR_COUNT    (FONT_LAST_CHAR - FONT_FIRST_CHAR + 1)

/* Text color palette indices in the BG palette */
#define TEXT_COLOR_WHITE    1
#define TEXT_COLOR_GRAY     2
#define TEXT_COLOR_RED      3
#define TEXT_COLOR_GREEN    4
#define TEXT_COLOR_BLUE     5
#define TEXT_COLOR_YELLOW   6
#define TEXT_COLOR_ORANGE   7
#define TEXT_COLOR_BLACK    8
#define TEXT_COLOR_DARK_GRAY 9

/* ---- Functions ---- */

/**
 * Initialize the text renderer.
 * Loads the font tile data into VRAM (BG tile block).
 * Sets up text colors in the palette.
 */
void text_init(void);

/**
 * Draw a string of text at tile position (tx, ty) on BG3.
 * @param tx    Tile X position (0..29)
 * @param ty    Tile Y position (0..19)
 * @param str   Null-terminated string
 * @param color Palette index for text color
 */
void text_draw(u8 tx, u8 ty, const char* str, u8 color);

/**
 * Draw a string of text at pixel position on BG3.
 * @param px    Pixel X position
 * @param py    Pixel Y position
 * @param str   Null-terminated string
 * @param color Palette index for text color
 */
void text_draw_px(u16 px, u16 py, const char* str, u8 color);

/**
 * Draw an integer number at tile position.
 * @param tx    Tile X position
 * @param ty    Tile Y position
 * @param num   Integer to render
 * @param color Palette index
 */
void text_draw_int(u8 tx, u8 ty, s32 num, u8 color);

/**
 * Draw an integer at pixel position.
 */
void text_draw_int_px(u16 px, u16 py, s32 num, u8 color);

/**
 * Clear a rectangular area of the text layer.
 * @param tx    Tile X
 * @param ty    Tile Y
 * @param w     Width in tiles
 * @param h     Height in tiles
 */
void text_clear_area(u8 tx, u8 ty, u8 w, u8 h);

/**
 * Clear the entire text layer.
 */
void text_clear_all(void);

/**
 * Draw a single character.
 * @param ch    ASCII character
 * @param tx    Tile X
 * @param ty    Tile Y
 * @param color Palette index
 */
void text_draw_char(char ch, u8 tx, u8 ty, u8 color);

/**
 * Flush the text tilemap buffer to VRAM.
 * Call after drawing text to make it visible on screen.
 */
void text_flush(void);

#endif /* TEXT_RENDERER_H */
