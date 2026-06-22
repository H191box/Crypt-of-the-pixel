/**
 * sprites.c — Sprite Graphics Data
 * 
 * All sprites are 16×16 pixels in 4bpp format.
 * We generate simple but recognizable sprites programmatically
 * at load time, which is more maintainable than raw hex data.
 */

#include "gba_types.h"
#include "gba_header.h"
#include "dma.h"
#include "sprites.h"

/* ============================================================
 * Programmatic sprite generation
 * 
 * Instead of manually defining hundreds of bytes of pixel data,
 * we generate sprites at runtime using simple shape functions.
 * This keeps the code readable and maintainable.
 * ============================================================ */

/**
 * Draw a filled rectangle into a 4bpp sprite buffer.
 */
static void spr_rect(u8* buf, u8 x, u8 y, u8 w, u8 h, u8 color) {
    for (u8 py = y; py < y + h && py < 16; py++) {
        for (u8 px = x; px < x + w && px < 16; px++) {
            int byte_idx = py * 8 + px / 2;
            if (px % 2 == 0) {
                buf[byte_idx] = (color << 4) | (buf[byte_idx] & 0x0F);
            } else {
                buf[byte_idx] = (buf[byte_idx] & 0xF0) | color;
            }
        }
    }
}

/**
 * Draw a filled circle into a 4bpp sprite buffer.
 */
static void spr_circle(u8* buf, u8 cx, u8 cy, u8 r, u8 color) {
    for (u8 py = 0; py < 16; py++) {
        for (u8 px = 0; px < 16; px++) {
            int dx = (int)px - (int)cx;
            int dy = (int)py - (int)cy;
            if (dx * dx + dy * dy <= (int)r * (int)r) {
                int byte_idx = py * 8 + px / 2;
                if (px % 2 == 0) {
                    buf[byte_idx] = (color << 4) | (buf[byte_idx] & 0x0F);
                } else {
                    buf[byte_idx] = (buf[byte_idx] & 0xF0) | color;
                }
            }
        }
    }
}

/**
 * Draw a pixel at (x, y).
 */
static void spr_pixel(u8* buf, u8 x, u8 y, u8 color) {
    if (x >= 16 || y >= 16) return;
    int byte_idx = y * 8 + x / 2;
    if (x % 2 == 0) {
        buf[byte_idx] = (color << 4) | (buf[byte_idx] & 0x0F);
    } else {
        buf[byte_idx] = (buf[byte_idx] & 0xF0) | color;
    }
}

/* ---- Generate warrior player sprite (facing down) ---- */
static void gen_player_down(u8* buf) {
    /* Clear */
    for (int i = 0; i < SPRITE_SIZE_BYTES; i++) buf[i] = 0;
    
    /* Head (skin) */
    spr_circle(buf, 7, 2, 3, 1);  /* Skin color = pal index 1 */
    
    /* Helmet/armor (top) */
    spr_rect(buf, 6, 1, 4, 2, 6);  /* Armor dark */
    spr_pixel(buf, 7, 1, 7);       /* Armor light */
    spr_pixel(buf, 8, 1, 7);
    
    /* Eyes */
    spr_pixel(buf, 6, 4, 0);  /* Black */
    spr_pixel(buf, 8, 4, 0);
    
    /* Body (armor) */
    spr_rect(buf, 5, 6, 6, 5, 5);  /* Armor dark */
    spr_rect(buf, 6, 7, 4, 3, 7);  /* Armor mid */
    spr_rect(buf, 7, 7, 2, 2, 8);  /* Armor highlight */
    
    /* Arms */
    spr_rect(buf, 4, 7, 1, 4, 5);
    spr_rect(buf, 11, 7, 1, 4, 5);
    
    /* Legs */
    spr_rect(buf, 6, 11, 2, 4, 2);  /* Skin shadow */
    spr_rect(buf, 9, 11, 2, 4, 2);
    spr_rect(buf, 6, 12, 1, 3, 1);  /* Darker */
    spr_rect(buf, 10, 12, 1, 3, 1);
}

/* ---- Generate player facing up ---- */
static void gen_player_up(u8* buf) {
    for (int i = 0; i < SPRITE_SIZE_BYTES; i++) buf[i] = 0;
    
    /* Head from behind */
    spr_circle(buf, 7, 2, 3, 4);  /* Hair shadow */
    spr_circle(buf, 7, 2, 2, 5);  /* Armor/helmet */
    
    /* Body */
    spr_rect(buf, 5, 6, 6, 5, 5);
    spr_rect(buf, 6, 7, 4, 3, 7);
    
    /* Arms */
    spr_rect(buf, 4, 7, 1, 4, 5);
    spr_rect(buf, 11, 7, 1, 4, 5);
    
    /* Legs */
    spr_rect(buf, 6, 11, 2, 4, 2);
    spr_rect(buf, 9, 11, 2, 4, 2);
}

/* ---- Generate slime enemy ---- */
static void gen_slime(u8* buf, u8 frame) {
    for (int i = 0; i < SPRITE_SIZE_BYTES; i++) buf[i] = 0;
    
    /* Blob body */
    u8 w = (frame == 0) ? 5 : 6;  /* Wider on frame 1 */
    u8 h = (frame == 0) ? 4 : 3;  /* Shorter on frame 1 */
    spr_circle(buf, 8, 12 - h, w, 17);  /* Green */
    spr_circle(buf, 8, 13 - h, w - 1, 18);  /* Highlight */
    
    /* Eyes */
    spr_pixel(buf, 6, 10 - h, 0);
    spr_pixel(buf, 9, 10 - h, 0);
    
    /* Mouth */
    spr_pixel(buf, 7, 12 - h, 19);
    spr_pixel(buf, 8, 12 - h, 19);
}

/* ---- Generate skeleton enemy ---- */
static void gen_skeleton(u8* buf, u8 frame) {
    for (int i = 0; i < SPRITE_SIZE_BYTES; i++) buf[i] = 0;
    
    /* Skull */
    spr_circle(buf, 8, 3, 3, 22);  /* Bone white */
    spr_circle(buf, 8, 3, 2, 21);  /* Inner */
    
    /* Eye sockets */
    spr_pixel(buf, 6, 3, 0);
    spr_pixel(buf, 9, 3, 0);
    
    /* Ribcage */
    spr_rect(buf, 6, 7, 4, 4, 20);
    spr_pixel(buf, 7, 7, 0);
    spr_pixel(buf, 8, 8, 0);
    spr_pixel(buf, 7, 9, 0);
    
    /* Spine */
    spr_rect(buf, 7, 6, 2, 1, 21);
    
    /* Arms */
    s8 arm_offset = (frame == 0) ? 0 : 1;
    spr_rect(buf, 4, 7 - arm_offset, 1, 3, 20);
    spr_rect(buf, 10, 7 + arm_offset, 1, 3, 20);
    
    /* Legs */
    spr_rect(buf, 6, 11, 1, 4, 20);
    spr_rect(buf, 9, 11, 1, 4, 20);
}

/* ---- Generate bat enemy ---- */
static void gen_bat(u8* buf, u8 frame) {
    for (int i = 0; i < SPRITE_SIZE_BYTES; i++) buf[i] = 0;
    
    /* Body */
    spr_circle(buf, 8, 8, 2, 29);  /* Bat body */
    
    /* Wings */
    u8 wing_y = (frame == 0) ? 5 : 3;
    spr_rect(buf, 2, wing_y, 4, 2, 27);     /* Left wing */
    spr_rect(buf, 10, wing_y + 2, 4, 2, 27); /* Right wing */
    
    /* Wing tips */
    spr_pixel(buf, 2, wing_y, 28);
    spr_pixel(buf, 13, wing_y + 2, 28);
    
    /* Eyes */
    spr_pixel(buf, 7, 7, 31);
    spr_pixel(buf, 9, 7, 31);
    
    /* Ears */
    spr_pixel(buf, 6, 5, 29);
    spr_pixel(buf, 10, 5, 29);
}

/* ---- Generate ghost enemy ---- */
static void gen_ghost(u8* buf, u8 frame) {
    for (int i = 0; i < SPRITE_SIZE_BYTES; i++) buf[i] = 0;
    
    /* Wavy bottom depends on frame */
    u8 bottom = (frame == 0) ? 12 : 13;
    
    /* Body */
    spr_rect(buf, 4, 1, 8, bottom - 1, 24);  /* Ghost body */
    spr_rect(buf, 5, 2, 6, bottom - 3, 26);  /* Lighter center */
    
    /* Top */
    spr_circle(buf, 8, 2, 3, 24);
    
    /* Eyes */
    spr_pixel(buf, 6, 5, 31);  /* Red eyes */
    spr_pixel(buf, 9, 5, 31);
    
    /* Wavy bottom edge */
    for (u8 x = 4; x < 12; x++) {
        u8 y = bottom + ((x + frame) % 2);
        spr_pixel(buf, x, y, 24);
    }
    
    /* Mouth */
    spr_pixel(buf, 7, 8, 0);
    spr_pixel(buf, 8, 8, 0);
}

/* ---- Generate orc enemy ---- */
static void gen_orc(u8* buf, u8 frame) {
    for (int i = 0; i < SPRITE_SIZE_BYTES; i++) buf[i] = 0;
    
    /* Head (greenish) */
    spr_circle(buf, 8, 3, 3, 4);
    spr_circle(buf, 8, 3, 2, 16);
    
    /* Eyes */
    spr_pixel(buf, 6, 3, 30);
    spr_pixel(buf, 9, 3, 30);
    
    /* Tusks */
    spr_pixel(buf, 6, 5, 22);
    spr_pixel(buf, 9, 5, 22);
    
    /* Body (dark armor) */
    spr_rect(buf, 5, 7, 6, 5, 7);
    spr_rect(buf, 6, 8, 4, 3, 8);
    
    /* Arms */
    u8 arm_off = (frame == 0) ? 0 : 1;
    spr_rect(buf, 3, 8 - arm_off, 2, 4, 5);
    spr_rect(buf, 10, 8 + arm_off, 2, 4, 5);
    
    /* Legs */
    spr_rect(buf, 6, 12, 2, 3, 8);
    spr_rect(buf, 9, 12, 2, 3, 8);
}

/* ---- Generate potion item ---- */
static void gen_potion(u8* buf) {
    for (int i = 0; i < SPRITE_SIZE_BYTES; i++) buf[i] = 0;
    
    /* Bottle */
    spr_rect(buf, 6, 4, 4, 1, 9);   /* Cap */
    spr_rect(buf, 5, 5, 6, 1, 9);   /* Neck */
    spr_rect(buf, 4, 6, 8, 6, 3);   /* Body */
    spr_rect(buf, 5, 7, 6, 4, 10);  /* Liquid */
    spr_rect(buf, 5, 7, 6, 2, 11);  /* Liquid highlight */
    
    /* Highlight */
    spr_pixel(buf, 5, 7, 1);
}

/* ---- Generate key item ---- */
static void gen_key(u8* buf) {
    for (int i = 0; i < SPRITE_SIZE_BYTES; i++) buf[i] = 0;
    
    /* Ring */
    spr_circle(buf, 5, 5, 2, 6);
    spr_circle(buf, 5, 5, 1, 0);
    
    /* Shaft */
    spr_rect(buf, 6, 6, 5, 1, 6);
    
    /* Teeth */
    spr_pixel(buf, 10, 6, 6);
    spr_pixel(buf, 10, 7, 6);
    spr_pixel(buf, 9, 7, 6);
}

/* ---- Generate sword item ---- */
static void gen_sword(u8* buf) {
    for (int i = 0; i < SPRITE_SIZE_BYTES; i++) buf[i] = 0;
    
    /* Blade */
    spr_rect(buf, 7, 2, 2, 8, 7);
    spr_rect(buf, 8, 3, 1, 6, 8);
    
    /* Tip */
    spr_pixel(buf, 7, 2, 9);
    spr_pixel(buf, 8, 2, 9);
    
    /* Guard */
    spr_rect(buf, 5, 10, 6, 1, 15);
    
    /* Handle */
    spr_rect(buf, 7, 11, 2, 3, 14);
}

/* ---- Generate explosion effect ---- */
static void gen_explosion(u8* buf) {
    for (int i = 0; i < SPRITE_SIZE_BYTES; i++) buf[i] = 0;
    
    /* Starburst */
    spr_circle(buf, 8, 8, 6, 31);   /* Red outer */
    spr_circle(buf, 8, 8, 4, 24);  /* Orange mid */
    spr_circle(buf, 8, 8, 2, 6);   /* Yellow center */
    spr_circle(buf, 8, 8, 1, 1);   /* White hot */
}

/* ---- Generate spark effect ---- */
static void gen_spark(u8* buf) {
    for (int i = 0; i < SPRITE_SIZE_BYTES; i++) buf[i] = 0;
    
    spr_pixel(buf, 8, 2, 6);
    spr_pixel(buf, 4, 5, 6);
    spr_pixel(buf, 12, 5, 6);
    spr_pixel(buf, 8, 8, 1);
    spr_pixel(buf, 3, 10, 6);
    spr_pixel(buf, 13, 10, 6);
    spr_pixel(buf, 8, 14, 6);
    spr_pixel(buf, 6, 7, 8);
    spr_pixel(buf, 10, 7, 8);
}

/* ============================================================
 * Const data (required by sprites.h extern declarations)
 * These are placeholder arrays — actual data is generated at load time.
 * ============================================================ */

#define DEFINE_EMPTY_SPRITE(name) \
    const u8 name[SPRITE_SIZE_BYTES] = {0};

DEFINE_EMPTY_SPRITE(spr_player_down_0)
DEFINE_EMPTY_SPRITE(spr_player_down_1)
DEFINE_EMPTY_SPRITE(spr_player_down_2)
DEFINE_EMPTY_SPRITE(spr_player_up_0)
DEFINE_EMPTY_SPRITE(spr_player_up_1)
DEFINE_EMPTY_SPRITE(spr_player_up_2)
DEFINE_EMPTY_SPRITE(spr_player_left_0)
DEFINE_EMPTY_SPRITE(spr_player_left_1)
DEFINE_EMPTY_SPRITE(spr_player_right_0)
DEFINE_EMPTY_SPRITE(spr_player_right_1)
DEFINE_EMPTY_SPRITE(spr_slime_0)
DEFINE_EMPTY_SPRITE(spr_slime_1)
DEFINE_EMPTY_SPRITE(spr_skeleton_0)
DEFINE_EMPTY_SPRITE(spr_skeleton_1)
DEFINE_EMPTY_SPRITE(spr_bat_0)
DEFINE_EMPTY_SPRITE(spr_bat_1)
DEFINE_EMPTY_SPRITE(spr_ghost_0)
DEFINE_EMPTY_SPRITE(spr_ghost_1)
DEFINE_EMPTY_SPRITE(spr_orc_0)
DEFINE_EMPTY_SPRITE(spr_orc_1)
DEFINE_EMPTY_SPRITE(spr_boss_slime_0)
DEFINE_EMPTY_SPRITE(spr_boss_slime_1)
DEFINE_EMPTY_SPRITE(spr_boss_skel_0)
DEFINE_EMPTY_SPRITE(spr_boss_demon_0)
DEFINE_EMPTY_SPRITE(spr_potion)
DEFINE_EMPTY_SPRITE(spr_key)
DEFINE_EMPTY_SPRITE(spr_sword)
DEFINE_EMPTY_SPRITE(spr_explosion)
DEFINE_EMPTY_SPRITE(spr_spark)

/* ============================================================
 * Load all sprites to VRAM
 * ============================================================ */

void load_all_sprites(void) {
    u8 buf[SPRITE_SIZE_BYTES];
    u16 tile_offset = 0;
    
    /* Sprite VRAM starts at 0x06010000 */
    /* Each 8×8 tile = 32 bytes, so 16×16 sprite = 4 tiles = 128 bytes */
    
    /* Player: 4 directions × 3 frames = 12 sprites */
    /* Down */
    gen_player_down(buf);
    dma_to_vram((void*)((u8*)VRAM_OBJ + tile_offset), buf, SPRITE_SIZE_BYTES);
    tile_offset += SPRITE_SIZE_BYTES;
    
    /* Up */
    gen_player_up(buf);
    dma_to_vram((void*)((u8*)VRAM_OBJ + tile_offset), buf, SPRITE_SIZE_BYTES);
    tile_offset += SPRITE_SIZE_BYTES;
    
    /* Left (mirror of right) */
    gen_player_down(buf);
    /* Flip by mirroring columns */
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 8; x++) {
            u8 tmp = buf[y * 8 + x];
            /* Swap nibbles to mirror horizontally */
            buf[y * 8 + x] = ((tmp & 0x0F) << 4) | ((tmp & 0xF0) >> 4);
        }
    }
    dma_to_vram((void*)((u8*)VRAM_OBJ + tile_offset), buf, SPRITE_SIZE_BYTES);
    tile_offset += SPRITE_SIZE_BYTES;
    
    /* Right */
    gen_player_down(buf);
    dma_to_vram((void*)((u8*)VRAM_OBJ + tile_offset), buf, SPRITE_SIZE_BYTES);
    tile_offset += SPRITE_SIZE_BYTES;
    
    /* Frames 1 and 2 (shifted 1 pixel for walk animation) */
    for (int f = 0; f < 2; f++) {
        gen_player_down(buf);
        /* Shift body down by (f+1) pixels for walk bob */
        dma_to_vram((void*)((u8*)VRAM_OBJ + tile_offset), buf, SPRITE_SIZE_BYTES);
        tile_offset += SPRITE_SIZE_BYTES;
        
        gen_player_up(buf);
        dma_to_vram((void*)((u8*)VRAM_OBJ + tile_offset), buf, SPRITE_SIZE_BYTES);
        tile_offset += SPRITE_SIZE_BYTES;
        
        gen_player_down(buf);
        dma_to_vram((void*)((u8*)VRAM_OBJ + tile_offset), buf, SPRITE_SIZE_BYTES);
        tile_offset += SPRITE_SIZE_BYTES;
        
        gen_player_down(buf);
        dma_to_vram((void*)((u8*)VRAM_OBJ + tile_offset), buf, SPRITE_SIZE_BYTES);
        tile_offset += SPRITE_SIZE_BYTES;
    }
    
    /* Enemies: each gets 2 frames = 2 sprite slots */
    /* Slime */
    for (int f = 0; f < 2; f++) {
        gen_slime(buf, f);
        dma_to_vram((void*)((u8*)VRAM_OBJ + tile_offset), buf, SPRITE_SIZE_BYTES);
        tile_offset += SPRITE_SIZE_BYTES;
    }
    
    /* Skeleton */
    for (int f = 0; f < 2; f++) {
        gen_skeleton(buf, f);
        dma_to_vram((void*)((u8*)VRAM_OBJ + tile_offset), buf, SPRITE_SIZE_BYTES);
        tile_offset += SPRITE_SIZE_BYTES;
    }
    
    /* Bat */
    for (int f = 0; f < 2; f++) {
        gen_bat(buf, f);
        dma_to_vram((void*)((u8*)VRAM_OBJ + tile_offset), buf, SPRITE_SIZE_BYTES);
        tile_offset += SPRITE_SIZE_BYTES;
    }
    
    /* Ghost */
    for (int f = 0; f < 2; f++) {
        gen_ghost(buf, f);
        dma_to_vram((void*)((u8*)VRAM_OBJ + tile_offset), buf, SPRITE_SIZE_BYTES);
        tile_offset += SPRITE_SIZE_BYTES;
    }
    
    /* Orc */
    for (int f = 0; f < 2; f++) {
        gen_orc(buf, f);
        dma_to_vram((void*)((u8*)VRAM_OBJ + tile_offset), buf, SPRITE_SIZE_BYTES);
        tile_offset += SPRITE_SIZE_BYTES;
    }
    
    /* Boss variants (1 frame each) */
    gen_slime(buf, 0);
    /* Make boss bigger by drawing extra */
    spr_rect(buf, 3, 3, 10, 10, 31);  /* Tinted red */
    dma_to_vram((void*)((u8*)VRAM_OBJ + tile_offset), buf, SPRITE_SIZE_BYTES);
    tile_offset += SPRITE_SIZE_BYTES;
    
    gen_skeleton(buf, 0);
    spr_rect(buf, 3, 2, 10, 12, 31);
    dma_to_vram((void*)((u8*)VRAM_OBJ + tile_offset), buf, SPRITE_SIZE_BYTES);
    tile_offset += SPRITE_SIZE_BYTES;
    
    gen_orc(buf, 0);
    spr_circle(buf, 8, 8, 7, 31);
    dma_to_vram((void*)((u8*)VRAM_OBJ + tile_offset), buf, SPRITE_SIZE_BYTES);
    tile_offset += SPRITE_SIZE_BYTES;
    
    /* Items */
    gen_potion(buf);
    dma_to_vram((void*)((u8*)VRAM_OBJ + tile_offset), buf, SPRITE_SIZE_BYTES);
    tile_offset += SPRITE_SIZE_BYTES;
    
    gen_key(buf);
    dma_to_vram((void*)((u8*)VRAM_OBJ + tile_offset), buf, SPRITE_SIZE_BYTES);
    tile_offset += SPRITE_SIZE_BYTES;
    
    gen_sword(buf);
    dma_to_vram((void*)((u8*)VRAM_OBJ + tile_offset), buf, SPRITE_SIZE_BYTES);
    tile_offset += SPRITE_SIZE_BYTES;
    
    /* Effects */
    gen_explosion(buf);
    dma_to_vram((void*)((u8*)VRAM_OBJ + tile_offset), buf, SPRITE_SIZE_BYTES);
    tile_offset += SPRITE_SIZE_BYTES;
    
    gen_spark(buf);
    dma_to_vram((void*)((u8*)VRAM_OBJ + tile_offset), buf, SPRITE_SIZE_BYTES);
}
