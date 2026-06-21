/**
 * tilemap.c — Tile Map Implementation
 * 
 * Stores the dungeon grid in EWRAM. Renders visible tiles
 * to VRAM tilemap buffers using the isometric projection.
 * 
 * Rendering approach:
 * We use BG0 for the floor layer. Each GBA tile is 8×8 pixels.
 * Our isometric tile is 16×8 pixels = 2 GBA tiles wide, 1 tall.
 * 
 * To render isometric tiles into a regular tilemap, we define
 * our floor isometric tiles as 2 consecutive GBA tiles in VRAM.
 * The tilemap entry uses tile_id * 2 for the left half and
 * tile_id * 2 + 1 for the right half.
 */

#include "gba_header.h"
#include "gba_types.h"
#include "iso_math.h"
#include "tilemap.h"
#include "system.h"

/* ---- Tilemap storage ---- */
static TileMap s_map;

/* ---- Tile attribute table ---- */
static const TileAttr s_tile_attrs[TILE_MAX] = {
    /* id, walk, trans, wall, dmg */
    {  0, 0, 0, 0, 0 },   /* VOID */
    {  1, 1, 1, 0, 0 },   /* FLOOR */
    {  2, 1, 1, 0, 0 },   /* FLOOR2 */
    {  3, 1, 1, 0, 0 },   /* FLOOR3 */
    {  4, 0, 0, 1, 0 },   /* WALL */
    {  5, 0, 0, 1, 0 },   /* WALL2 */
    {  6, 1, 0, 0, 0 },   /* DOOR */
    {  7, 1, 1, 0, 0 },   /* STAIRS */
    {  8, 0, 1, 0, 1 },   /* WATER */
    {  9, 1, 1, 0, 0 },   /* CHEST */
    { 10, 1, 1, 0, 1 },   /* TRAP */
    { 11, 0, 1, 1, 0 },   /* TORCH */
    { 12, 1, 1, 0, 0 },   /* ALTAR */
    { 13, 1, 1, 0, 0 },   /* BONES */
};

/**
 * Initialize tilemap — zero out all tiles.
 */
void tilemap_init(void) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            s_map.tiles[y][x] = TILE_VOID;
            s_map.revealed[y][x] = 0;
            s_map.visible[y][x] = 0;
        }
    }
}

/**
 * Get pointer to the tilemap.
 */
TileMap* tilemap_get(void) {
    return &s_map;
}

/**
 * Set tile at grid position.
 */
void tilemap_set_tile(u8 x, u8 y, u8 id) {
    if (x >= MAP_WIDTH || y >= MAP_HEIGHT) return;
    s_map.tiles[y][x] = id;
}

/**
 * Get tile at grid position.
 */
u8 tilemap_get_tile(u8 x, u8 y) {
    if (x >= MAP_WIDTH || y >= MAP_HEIGHT) return TILE_VOID;
    return s_map.tiles[y][x];
}

/**
 * Check if a tile is walkable.
 */
bool tilemap_is_walkable(u8 x, u8 y) {
    if (x >= MAP_WIDTH || y >= MAP_HEIGHT) return FALSE;
    u8 id = s_map.tiles[y][x];
    if (id >= TILE_MAX) return FALSE;
    return s_tile_attrs[id].walkable ? TRUE : FALSE;
}

/**
 * Check if a tile is a wall.
 */
bool tilemap_is_wall(u8 x, u8 y) {
    if (x >= MAP_WIDTH || y >= MAP_HEIGHT) return FALSE;
    u8 id = s_map.tiles[y][x];
    if (id >= TILE_MAX) return FALSE;
    return s_tile_attrs[id].wall ? TRUE : FALSE;
}

/**
 * Get attributes for a tile type.
 */
TileAttr tilemap_get_attr(u8 tile_id) {
    if (tile_id >= TILE_MAX) return s_tile_attrs[TILE_VOID];
    return s_tile_attrs[tile_id];
}

/**
 * Reveal a tile (fog of war).
 */
void tilemap_reveal(u8 x, u8 y) {
    if (x >= MAP_WIDTH || y >= MAP_HEIGHT) return;
    s_map.revealed[y][x] = 1;
}

/**
 * Check if tile has been revealed.
 */
bool tilemap_is_revealed(u8 x, u8 y) {
    if (x >= MAP_WIDTH || y >= MAP_HEIGHT) return FALSE;
    return s_map.revealed[y][x] ? TRUE : FALSE;
}

/**
 * Set tile visibility for this frame.
 */
void tilemap_set_visible(u8 x, u8 y, bool vis) {
    if (x >= MAP_WIDTH || y >= MAP_HEIGHT) return;
    s_map.visible[y][x] = vis ? 1 : 0;
}

/**
 * Check if tile is currently visible.
 */
bool tilemap_is_visible(u8 x, u8 y) {
    if (x >= MAP_WIDTH || y >= MAP_HEIGHT) return FALSE;
    return s_map.visible[y][x] ? TRUE : FALSE;
}

/**
 * Clear all visibility flags.
 */
void tilemap_clear_visibility(void) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            s_map.visible[y][x] = 0;
        }
    }
}

/**
 * Simple radius-based FOV reveal.
 * Uses diamond-shaped radius (Chebyshev distance) for isometric feel.
 * Walls block visibility beyond them.
 */
void tilemap_reveal_around(u8 cx, u8 cy, u8 radius) {
    for (s8 dy = -(s8)radius; dy <= (s8)radius; dy++) {
        for (s8 dx = -(s8)radius; dx <= (s8)radius; dx++) {
            /* Diamond distance check: |dx| + |dy| <= radius */
            if (ABS(dx) + ABS(dy) > radius) continue;
            
            u8 tx = (u8)((s16)cx + dx);
            u8 ty = (u8)((s16)cy + dy);
            
            if (tx >= MAP_WIDTH || ty >= MAP_HEIGHT) continue;
            
            /* Check line of sight — simple wall blocking */
            /* Cast from center to this tile; if any wall in between, skip */
            bool blocked = FALSE;
            s16 steps = MAX(ABS(dx), ABS(dy));
            if (steps > 0) {
                for (s16 s = 1; s < steps; s++) {
                    s8 check_x = cx + (s8)(dx * s / steps);
                    s8 check_y = cy + (s8)(dy * s / steps);
                    if (tilemap_is_wall(check_x, check_y)) {
                        blocked = TRUE;
                        break;
                    }
                }
            }
            
            if (!blocked) {
                s_map.revealed[ty][tx] = 1;
                s_map.visible[ty][tx] = 1;
            } else {
                /* Reveal the tile but mark as not currently visible */
                s_map.revealed[ty][tx] = 1;
            }
        }
    }
}

/**
 * Render visible tilemap to BG layers.
 * 
 * Strategy: Walk through all visible/revealed tiles that would appear
 * on screen. For each, write the appropriate tile ID to the BG0
 * tilemap buffer in VRAM.
 * 
 * The BG0 tilemap is at screen base 28 (0xE000), which gives us
 * a 32×32 tile screen (256×256 pixel virtual space).
 * 
 * For isometric rendering: we convert each grid tile to screen coords
 * and write to the corresponding position in the BG tilemap.
 * Since isometric tiles don't align to the regular grid, we use
 * individual tile placement.
 * 
 * Each isometric floor tile occupies 2×1 GBA tiles (16×8).
 */
void tilemap_render(void) {
    /* 
     * For efficiency, we calculate the range of grid tiles that could
     * be visible on screen, then iterate only those.
     */
    
    IsoCamera* cam = iso_get_camera();
    
    /* Find the approximate grid range visible on screen.
     * Screen goes from (0,0) to (SCREEN_WIDTH, SCREEN_HEIGHT).
     * Grid coord 0,0 maps to: sx = 0*8 - 0*8 + CX + cam.x = CX + cam.x
     *                           sy = 0*4 + 0*4 + CY + cam.y = CY + cam.y
     * We solve for the grid extremes visible on screen.
     */
    
    /* Use the screen corners to find grid bounds */
    s16 min_gx, min_gy, max_gx, max_gy;
    s16 sx, sy;
    
    /* Top-left corner (0, 0) */
    screen_to_iso(0, 0, &min_gx, &min_gy);
    
    /* Bottom-right corner (SCREEN_WIDTH-1, SCREEN_HEIGHT-1) */
    screen_to_iso(SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, &max_gx, &max_gy);
    
    /* Expand range by 1 tile for safety */
    min_gx -= 2; min_gy -= 2;
    max_gx += 2; max_gy += 2;
    
    /* Clamp to map bounds */
    if (min_gx < 0) min_gx = 0;
    if (min_gy < 0) min_gy = 0;
    if (max_gx >= MAP_WIDTH) max_gx = MAP_WIDTH - 1;
    if (max_gy >= MAP_HEIGHT) max_gy = MAP_HEIGHT - 1;
    
    /* BG0 tilemap at VRAM screen base 28 = offset 0xE000 from VRAM start */
    /* Each entry is a 16-bit SE (screen entry): palette | tile_id */
    u16* bg0_map = (u16*)(VRAM + 0xE000);
    
    /* Clear the tilemap area we'll use (32x32 screen entries) */
    for (int i = 0; i < 32 * 32; i++) {
        bg0_map[i] = 0;  /* Tile 0 = transparent/void */
    }
    
    /* Render each tile */
    for (s16 gy = min_gy; gy <= max_gy; gy++) {
        for (s16 gx = min_gx; gx <= max_gx; gx++) {
            /* Only render revealed tiles */
            if (!s_map.revealed[gy][gx]) continue;
            
            u8 tile_id = s_map.tiles[gy][gx];
            if (tile_id == TILE_VOID) continue;
            
            /* Convert to screen coords */
            iso_to_screen(gx, gy, &sx, &sy);
            
            /* Convert screen pixel position to tilemap entry position.
             * BG tilemap entry (tx, ty) covers pixel area (tx*8, ty*8) to (tx*8+7, ty*8+7).
             */
            s16 map_x = sx >> 3;  /* Divide by 8 (tile size) */
            s16 map_y = sy >> 3;
            
            /* Skip if off the 32×32 tilemap */
            if (map_x < 0 || map_x >= 31 || map_y < 0 || map_y >= 32) continue;
            
            /* Darken non-visible (but revealed) tiles */
            /* We encode this as tile_id * 2 (visible) or tile_id * 2 + offset (dark) */
            u16 se;
            
            if (s_map.visible[gy][gx]) {
                /* Visible: use normal tile graphics */
                /* Each isometric tile is 2 GBA tiles wide */
                se = tile_id * 2;  /* Left half of isometric tile */
                bg0_map[map_y * 32 + map_x] = se;
                se = tile_id * 2 + 1;  /* Right half */
                bg0_map[map_y * 32 + map_x + 1] = se;
            } else {
                /* Previously seen but not currently visible: use dark variant */
                /* Add TILE_MAX * 2 to index into the dark tile graphics area */
                se = (tile_id + TILE_MAX) * 2;
                bg0_map[map_y * 32 + map_x] = se;
                se = (tile_id + TILE_MAX) * 2 + 1;
                bg0_map[map_y * 32 + map_x + 1] = se;
            }
        }
    }
    
    /* Update BG0 scroll to offset camera.
     * Since we center the tilemap at screen center, we set the
     * BG scroll to 0 (the iso_to_screen already accounts for camera).
     * However, the BG tilemap origin is at (0,0), so we need to offset
     * to center our rendering area. */
    REG_BG0HOFS = 0;
    REG_BG0VOFS = 0;
}
