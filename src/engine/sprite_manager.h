/**
 * sprite_manager.h — GBA Sprite Management System
 * 
 * Manages allocation, rendering, and animation of hardware sprites.
 * The GBA supports 128 OAM entries. We track allocations to avoid
 * conflicts. For isometric view, we need depth sorting.
 * 
 * Sprites used in this game:
 *   Player (up to 1, 16×16 pixels, 4bpp, 3 frames × 4 directions)
 *   Enemies (up to ~20, 16×16, 4bpp, 2 frames)
 *   Items on ground (up to ~10, 16×16, 4bpp)
 *   Effects (up to ~5, 16×16, 4bpp)
 *   Total: ~36 sprites max
 */

#ifndef SPRITE_MANAGER_H
#define SPRITE_MANAGER_H

#include "gba_types.h"

/* ---- Sprite limits ---- */
#define MAX_SPRITES         64
#define MAX_ANIM_FRAMES     4

/* ---- Sprite shapes ---- */
typedef enum {
    SPR_SHAPE_SQUARE = 0,  /* 8×8, 16×16, 32×32, or 64×64 */
    SPR_SHAPE_WIDE   = 1,  /* 16×8, 32×16, 64×32 */
    SPR_SHAPE_TALL   = 2   /* 8×16, 16×32, 32×64 */
} SpriteShape;

/* ---- Sprite sizes ---- */
typedef enum {
    SPR_SIZE_8x8   = 0,
    SPR_SIZE_16x16  = 1,
    SPR_SIZE_32x32  = 2,
    SPR_SIZE_64x64  = 3
} SpriteSize;

/* ---- Animation configuration ---- */
typedef struct {
    u8 frame_count;            /* Total frames in animation */
    u8 frames[MAX_ANIM_FRAMES];/* Tile index for each frame */
    u8 speed;                  /* Frames to wait between animation steps */
} SpriteAnim;

/* ---- Sprite instance ---- */
typedef struct {
    bool active;       /* Is this sprite slot in use? */
    u16 attr0;         /* OAM Attribute 0: Y, shape, color mode, etc. */
    u16 attr1;         /* OAM Attribute 1: X, flip, size */
    u16 attr2;         /* OAM Attribute 2: tile index, priority, palette */
    
    /* Game-level state */
    s16 x;             /* Position X (screen pixels) */
    s16 y;             /* Position Y (screen pixels) */
    u8 tile_idx;       /* Base tile index in sprite tile data */
    u8 palette;         /* Palette bank (0-15 for 16-color sprites) */
    u8 priority;        /* Draw priority (0=highest/behind, 3=lowest/in-front) */
    bool visible;       /* Show/hide */
    
    /* Animation state */
    u8 anim_frame;      /* Current frame index */
    u8 anim_timer;      /* Timer for animation stepping */
    SpriteAnim* anim;   /* Pointer to animation data (NULL = static) */
    
    /* Depth for sorting (y position in isometric space) */
    u16 depth;
} Sprite;

/* ---- Functions ---- */

/**
 * Initialize the sprite manager.
 * Clears all OAM entries and resets the allocation table.
 */
void sprite_init(void);

/**
 * Create a new sprite.
 * @param x        Screen X position
 * @param y        Screen Y position
 * @param tile_idx Base tile index in sprite VRAM
 * @param shape    Sprite shape
 * @param size     Sprite size
 * @param palette  Palette number (0-15)
 * @param priority Draw priority (0-3)
 * @return Sprite slot index, or -1 if no slots available.
 */
s8 sprite_create(s16 x, s16 y, u8 tile_idx, SpriteShape shape, 
                  SpriteSize size, u8 palette, u8 priority);

/**
 * Destroy a sprite (free its slot).
 */
void sprite_destroy(s8 id);

/**
 * Set sprite position.
 */
void sprite_set_pos(s8 id, s16 x, s16 y);

/**
 * Set sprite tile (change graphic).
 */
void sprite_set_tile(s8 id, u8 tile_idx);

/**
 * Show/hide a sprite.
 */
void sprite_set_visible(s8 id, bool visible);

/**
 * Set sprite priority for draw order.
 */
void sprite_set_priority(s8 id, u8 priority);

/**
 * Set depth for isometric sorting (higher depth = drawn later = on top).
 */
void sprite_set_depth(s8 id, u16 depth);

/**
 * Start an animation on a sprite.
 * @param id    Sprite slot
 * @param anim  Pointer to animation config
 */
void sprite_animate(s8 id, SpriteAnim* anim);

/**
 * Stop animation, show static frame.
 */
void sprite_stop_anim(s8 id);

/**
 * Update all active sprites.
 * Advances animations, prepares OAM data.
 * Call once per frame before sprite_render().
 */
void sprite_update(void);

/**
 * Write all active sprites to OAM via DMA.
 * Also performs depth sorting for isometric rendering.
 */
void sprite_render(void);

/**
 * Get a sprite instance by ID.
 * @return Pointer to sprite data, or NULL if invalid.
 */
Sprite* sprite_get(s8 id);

/**
 * Hide all sprites (for transitions).
 */
void sprite_hide_all(void);

/**
 * Copy sprite tile data to VRAM.
 * @param data     Source tile pixel data (4bpp)
 * @param tile_idx Starting tile index in sprite VRAM
 * @param tiles    Number of 8×8 tiles to copy
 * @param bytes    Size of data in bytes
 */
void sprite_load_tiles(const u8* data, u16 tile_idx, u16 tiles, u32 bytes);

/**
 * Load sprite palette.
 */
void sprite_load_palette(const u16* pal, u16 count);

#endif /* SPRITE_MANAGER_H */
