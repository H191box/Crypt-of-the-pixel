/**
 * sprite_manager.c — GBA Sprite Management Implementation
 * 
 * Manages a pool of up to 64 sprite slots. Each frame, active
 * sprites are depth-sorted (for isometric rendering) and written
 * to OAM via DMA.
 * 
 * OAM Layout (per sprite, 6 bytes):
 *   Attr0 (2 bytes): Y(8) | Shape(2) | ColorMode(1) | Mosaic(1) | FlipModes(2) | Size(2)
 *   Attr1 (2 bytes): X(9) | HFlip(1) | VFlip(1) | AffineIndex(5)
 *   Attr2 (2 bytes): TileIndex(10) | Priority(2) | PaletteBank(4)
 */

#include "gba_header.h"
#include "gba_types.h"
#include "sprite_manager.h"
#include "dma.h"

/* ---- Sprite pool ---- */
static Sprite s_sprites[MAX_SPRITES];

/* ---- Internal OAM buffer (written to OAM via DMA) ---- */
/* 128 sprites × 3 attributes × 2 bytes = 768 bytes */
/* Plus 16 affine parameter sets × 4 params × 2 bytes = 128 bytes */
static u32 s_oam_buffer[128];

/* ---- Initialize ---- */
void sprite_init(void) {
    for (int i = 0; i < MAX_SPRITES; i++) {
        s_sprites[i].active = FALSE;
        s_sprites[i].visible = FALSE;
        s_sprites[i].x = -64;
        s_sprites[i].y = -64;
        s_sprites[i].attr0 = 0;
        s_sprites[i].attr1 = 0;
        s_sprites[i].attr2 = 0;
        s_sprites[i].anim = NULL;
        s_sprites[i].anim_frame = 0;
        s_sprites[i].anim_timer = 0;
        s_sprites[i].depth = 0;
    }
    
    /* Clear OAM buffer */
    for (int i = 0; i < 128; i++) {
        s_oam_buffer[i] = 0;
    }
}

/**
 * Create a new sprite.
 */
s8 sprite_create(s16 x, s16 y, u8 tile_idx, SpriteShape shape,
                  SpriteSize size, u8 palette, u8 priority) {
    /* Find free slot */
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (!s_sprites[i].active) {
            Sprite* spr = &s_sprites[i];
            spr->active = TRUE;
            spr->visible = TRUE;
            spr->x = x;
            spr->y = y;
            spr->tile_idx = tile_idx;
            spr->palette = palette & 0xF;
            spr->priority = priority & 0x3;
            spr->anim = NULL;
            spr->anim_frame = 0;
            spr->anim_timer = 0;
            spr->depth = (u16)y;  /* Default depth = screen Y */
            
            /* Build OAM attributes */
            /* Attr0: Y(8 bits), shape(2 bits at bits 14-13), no mosaic, no flip */
            spr->attr0 = ((u16)y & 0xFF) | ((u16)shape << 14);
            /* Attr1: X(9 bits at bits 8-0), size(2 bits at bits 14-13) */
            spr->attr1 = ((u16)x & 0x1FF) | ((u16)size << 12);
            /* Attr2: tile(10 bits), priority(2 bits at 12-13), palette(4 bits at 12-15 for 16-color) */
            spr->attr2 = (u16)tile_idx | ((u16)priority << 10) | ((u16)palette << 12);
            
            return (s8)i;
        }
    }
    return -1;  /* No free slots */
}

/**
 * Destroy a sprite.
 */
void sprite_destroy(s8 id) {
    if (id < 0 || id >= MAX_SPRITES) return;
    s_sprites[id].active = FALSE;
    s_sprites[id].visible = FALSE;
    /* Set Y off-screen to hide */
    s_sprites[id].attr0 = 0x8000;  /* Y > 159 means off-screen */
}

/**
 * Set sprite position.
 */
void sprite_set_pos(s8 id, s16 x, s16 y) {
    if (id < 0 || id >= MAX_SPRITES) return;
    Sprite* spr = &s_sprites[id];
    spr->x = x;
    spr->y = y;
    
    /* Update OAM attributes */
    spr->attr0 = (spr->attr0 & 0xFF00) | ((u16)y & 0x00FF);
    spr->attr1 = (spr->attr1 & 0xFE00) | ((u16)x & 0x01FF);
}

/**
 * Set sprite tile index.
 */
void sprite_set_tile(s8 id, u8 tile_idx) {
    if (id < 0 || id >= MAX_SPRITES) return;
    s_sprites[id].tile_idx = tile_idx;
    s_sprites[id].attr2 = (s_sprites[id].attr2 & 0xFC00) | tile_idx;
}

/**
 * Show/hide sprite.
 */
void sprite_set_visible(s8 id, bool visible) {
    if (id < 0 || id >= MAX_SPRITES) return;
    s_sprites[id].visible = visible;
    if (!visible) {
        /* Move off-screen */
        s_sprites[id].attr0 = 0x8000;
    } else {
        s_sprites[id].attr0 = (s_sprites[id].attr0 & 0xFF00) | 
                              ((u16)s_sprites[id].y & 0x00FF);
    }
}

/**
 * Set sprite priority.
 */
void sprite_set_priority(s8 id, u8 priority) {
    if (id < 0 || id >= MAX_SPRITES) return;
    s_sprites[id].priority = priority & 0x3;
    s_sprites[id].attr2 = (s_sprites[id].attr2 & 0x3FFF) | 
                           ((u16)(priority & 0x3) << 10);
}

/**
 * Set depth for isometric sorting.
 */
void sprite_set_depth(s8 id, u16 depth) {
    if (id < 0 || id >= MAX_SPRITES) return;
    s_sprites[id].depth = depth;
}

/**
 * Start animation.
 */
void sprite_animate(s8 id, SpriteAnim* anim) {
    if (id < 0 || id >= MAX_SPRITES) return;
    s_sprites[id].anim = anim;
    s_sprites[id].anim_frame = 0;
    s_sprites[id].anim_timer = 0;
    if (anim && anim->frame_count > 0) {
        sprite_set_tile(id, anim->frames[0]);
    }
}

/**
 * Stop animation.
 */
void sprite_stop_anim(s8 id) {
    if (id < 0 || id >= MAX_SPRITES) return;
    s_sprites[id].anim = NULL;
}

/**
 * Bubble sort for sprite depth ordering.
 * Sorts active sprites by depth (ascending).
 * We only sort the order we write to OAM, not the sprite pool itself.
 */
static void sort_sprites_by_depth(s8* order, u8 count) {
    for (u8 i = 0; i < count - 1; i++) {
        for (u8 j = 0; j < count - i - 1; j++) {
            s8 a = order[j];
            s8 b = order[j + 1];
            if (s_sprites[a].depth > s_sprites[b].depth) {
                order[j] = b;
                order[j + 1] = a;
            }
        }
    }
}

/**
 * Update all sprites (advance animations).
 */
void sprite_update(void) {
    for (int i = 0; i < MAX_SPRITES; i++) {
        Sprite* spr = &s_sprites[i];
        if (!spr->active || !spr->visible) continue;
        if (!spr->anim) continue;
        
        spr->anim_timer++;
        if (spr->anim_timer >= spr->anim->speed) {
            spr->anim_timer = 0;
            spr->anim_frame++;
            if (spr->anim_frame >= spr->anim->frame_count) {
                spr->anim_frame = 0;  /* Loop */
            }
            sprite_set_tile(i, spr->anim->frames[spr->anim_frame]);
        }
    }
}

/**
 * Render sprites to OAM.
 * 1. Collect active sprites
 * 2. Sort by depth (for isometric)
 * 3. Write to OAM buffer
 * 4. DMA to hardware OAM
 */
void sprite_render(void) {
    /* Collect active sprite IDs */
    s8 order[MAX_SPRITES];
    u8 count = 0;
    
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (s_sprites[i].active && s_sprites[i].visible) {
            order[count++] = (s8)i;
        }
    }
    
    /* Sort by depth */
    sort_sprites_by_depth(order, count);
    
    /* Write sorted sprites to OAM buffer */
    for (u8 i = 0; i < count; i++) {
        Sprite* spr = &s_sprites[order[i]];
        
        /* Each sprite is 3 u16 attributes = 1.5 u32.
         * We pack attr0, attr1 as one u32 and attr2 + next attr0 as next.
         * Actually, OAM expects: attr0 at +0, attr1 at +2, attr2 at +4.
         * So sprite i's data starts at byte offset i*6. */
        u16* oam16 = (u16*)s_oam_buffer;
        u16 base = i * 6;  /* 6 bytes per sprite */
        
        oam16[base + 0] = spr->attr0;
        oam16[base + 1] = spr->attr1;
        oam16[base + 2] = spr->attr2;
    }
    
    /* Hide any remaining OAM entries beyond our active count */
    for (u8 i = count; i < 128; i++) {
        u16* oam16 = (u16*)s_oam_buffer;
        oam16[i * 6 + 0] = 0x8000;  /* Y > 159 = off screen */
        oam16[i * 6 + 1] = 0;
        oam16[i * 6 + 2] = 0;
    }
    
    /* DMA OAM buffer to hardware */
    dma_to_oam(s_oam_buffer);
}

/**
 * Get sprite pointer.
 */
Sprite* sprite_get(s8 id) {
    if (id < 0 || id >= MAX_SPRITES) return NULL;
    return &s_sprites[id];
}

/**
 * Hide all sprites.
 */
void sprite_hide_all(void) {
    for (int i = 0; i < MAX_SPRITES; i++) {
        s_sprites[i].visible = FALSE;
        s_sprites[i].attr0 = 0x8000;
    }
}

/**
 * Load sprite tile data to VRAM.
 */
void sprite_load_tiles(const u8* data, u16 tile_idx, u16 tiles, u32 bytes) {
    /* Sprite VRAM starts at 0x06010000.
     * Each 4bpp tile is 32 bytes (8×8 pixels × 4 bits/pixel).
     * tile_idx * 32 gives the offset. */
    void* dst = (void*)(VRAM_OBJ + tile_idx * 32);
    dma_to_vram(dst, data, bytes);
}

/**
 * Load sprite palette.
 */
void sprite_load_palette(const u16* pal, u16 count) {
    dma_to_palette(pal, count, FALSE);
}
