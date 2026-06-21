/**
 * font.h — 8×8 Bitmap Font Data
 * 
 * Full printable ASCII: characters 32 (space) through 126 (~).
 * 95 characters × 8 bytes each = 760 bytes.
 * Each character is 8×8 pixels, 1bpp (monochrome).
 * Stored as byte-per-row: each bit = one pixel (MSB = leftmost).
 */

#ifndef FONT_H
#define FONT_H

#include "gba_types.h"

#define FONT_CHAR_COUNT  95
#define FONT_DATA_SIZE   (FONT_CHAR_COUNT * 8)  /* 760 bytes */

/* ---- Font data (1bpp, 8 bytes per character) ---- */
extern const u8 font_data[FONT_DATA_SIZE];

/* ---- Get converted 4bpp font tiles ---- */
/* Returns pointer to 95×32 byte array of 4bpp tile data */
const u8* font_get_tiles(void);

#endif /* FONT_H */
