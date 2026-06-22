/**
 * text_renderer.c — Text Rendering Implementation
 * 
 * Loads the built-in 8×8 font into VRAM as BG tiles.
 * Renders text by writing tile indices into the BG3 tilemap.
 * Each character uses its own palette-indexed tile.
 * 
 * Font tile layout in VRAM (BG char block):
 *   Tiles FONT_TILE_BASE + 0..95 for color 0 (white on black)
 *   We allocate 16 color variants of each character:
 *     variant_base = FONT_TILE_BASE + char_idx * 16 + color_idx
 *   Total: 95 chars × 16 colors = 1520 tiles (not all needed)
 *   Simplified: we just swap palette banks per character.
 * 
 * Actually, simpler approach:
 *   Store font tiles once. For colored text, we write the same
 *   tile index but set different palette bits in the screen entry.
 *   Each 16-color palette bank can define a different text color.
 */

#include "gba_header.h"
#include "gba_types.h"
#include "text_renderer.h"
#include "dma.h"

/* ---- Font tile data is in font.c ---- */
extern const u16 font_palette[];    /* Text palette: 16 colors */

/* Get font tiles from font.c (generated at runtime from 1bpp) */
extern const u8* font_get_tiles(void);

/* ---- VRAM locations ---- */
/* BG3 uses char block 0 and screen block 31 (0xF800) */
/* Font tiles start at offset 512 tiles into char block 0 (offset 0x4000)
 * to leave room for game tile graphics. */
#define FONT_TILE_BASE     512  /* Tile index in char block 0 */
#define TEXT_MAP_BASE      31   /* Screen block for BG3 */
#define TEXT_MAP_VRAM      ((u16*)((u8*)VRAM + 0xF800))

/* ---- Internal buffer for text tilemap ---- */
/* BG3 screen block is 32×32 = 1024 entries = 2048 bytes */
static u16 s_text_map[32 * 32];

/**
 * Initialize text renderer.
 * Copies font tile data to VRAM and sets up text palette.
 */
void text_init(void) {
    /* Copy font tiles to VRAM at FONT_TILE_BASE */
    const u8* tiles = font_get_tiles();
    void* tile_dst = (void*)((u8*)VRAM + FONT_TILE_BASE * 32);
    dma_to_vram(tile_dst, tiles, FONT_CHAR_COUNT * 32);
    
    /* Copy font palette to palette bank 0 of BG palette */
    /* Actually, we'll use palette bank 15 for text so it doesn't interfere */
    /* BG palette has 16 banks × 16 colors = 256 colors total.
     * Bank n = PAL_BG[n*16 .. n*16+15] */
    for (int i = 0; i < 16; i++) {
        PAL_BG[i] = font_palette[i];
    }
    
    /* Clear text tilemap */
    text_clear_all();
}

/**
 * Draw a string at tile coordinates.
 */
void text_draw(u8 tx, u8 ty, const char* str, u8 color) {
    if (!str) return;
    
    u8 x = tx;
    while (*str) {
        char ch = *str;
        str++;
        
        if (ch == '\n') {
            ty++;
            x = tx;
            continue;
        }
        
        if (ch < FONT_FIRST_CHAR || ch > FONT_LAST_CHAR) {
            x++;
            continue;
        }
        
        u8 char_idx = (u8)(ch - FONT_FIRST_CHAR);
        
        /* Screen entry: tile index (10 bits) | palette bank (4 bits) */
        /* For text, we use palette bank 0 (same bank, different color via index) */
        u16 tile = FONT_TILE_BASE + char_idx;
        u16 se = tile | (color << 12);  /* Palette bank = color */
        
        if (x < 32 && ty < 32) {
            s_text_map[ty * 32 + x] = se;
        }
        
        x++;
    }
}

/**
 * Draw text at pixel coordinates.
 */
void text_draw_px(u16 px, u16 py, const char* str, u8 color) {
    u8 tx = (u8)(px / 8);
    u8 ty = (u8)(py / 8);
    text_draw(tx, ty, str, color);
}

/**
 * Draw an integer at tile position.
 */
void text_draw_int(u8 tx, u8 ty, s32 num, u8 color) {
    char buf[12];
    s32 i = 0;
    bool neg = FALSE;
    
    if (num < 0) {
        neg = TRUE;
        num = -num;
    }
    
    if (num == 0) {
        buf[i++] = '0';
    } else {
        while (num > 0) {
            buf[i++] = '0' + (num % 10);
            num /= 10;
        }
    }
    
    /* Reverse */
    char out[12];
    u8 oi = 0;
    if (neg) out[oi++] = '-';
    while (i > 0) {
        out[oi++] = buf[--i];
    }
    out[oi] = '\0';
    
    text_draw(tx, ty, out, color);
}

/**
 * Draw an integer at pixel position.
 */
void text_draw_int_px(u16 px, u16 py, s32 num, u8 color) {
    u8 tx = (u8)(px / 8);
    u8 ty = (u8)(py / 8);
    text_draw_int(tx, ty, num, color);
}

/**
 * Clear a rectangular text area.
 */
void text_clear_area(u8 tx, u8 ty, u8 w, u8 h) {
    for (u8 y = ty; y < ty + h && y < 32; y++) {
        for (u8 x = tx; x < tx + w && x < 32; x++) {
            s_text_map[y * 32 + x] = 0;  /* Tile 0 = blank */
        }
    }
}

/**
 * Clear entire text layer.
 */
void text_clear_all(void) {
    for (int i = 0; i < 32 * 32; i++) {
        s_text_map[i] = 0;
    }
}

/**
 * Draw a single character.
 */
void text_draw_char(char ch, u8 tx, u8 ty, u8 color) {
    char buf[2] = { ch, '\0' };
    text_draw(tx, ty, buf, color);
}

/* Note: text_flush() is called by the HUD system to write s_text_map to VRAM.
 * We expose it here for the HUD to use. */
void text_flush(void) {
    dma_copy_16(TEXT_MAP_VRAM, s_text_map, 32 * 32);
}
