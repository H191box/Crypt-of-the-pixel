/**
 * tilemap.h — Tile Map System
 * 
 * Manages the dungeon tile grid. Each tile is a u8 ID that maps
 * to graphics data, walkability, and other properties.
 * 
 * Map size: 64×64 tiles = 4096 bytes (fits easily in EWRAM).
 * 
 * Rendering: Only draws tiles visible on screen (viewport culling).
 * Uses BG0 for floor tiles and BG1 for wall overlay tiles.
 */

#ifndef TILEMAP_H
#define TILEMAP_H

#include "gba_types.h"
#include "iso_math.h"  /* MAP_WIDTH, MAP_HEIGHT */

/* ---- Tile IDs ---- */
enum TileID {
    TILE_VOID     = 0,   /* Empty / not generated */
    TILE_FLOOR    = 1,   /* Stone floor */
    TILE_FLOOR2   = 2,   /* Alternate stone floor */
    TILE_FLOOR3   = 3,   /* Cracked floor */
    TILE_WALL     = 4,   /* Wall (impassable) */
    TILE_WALL2    = 5,   /* Alternate wall */
    TILE_DOOR     = 6,   /* Door (opened/closed) */
    TILE_STAIRS   = 7,   /* Stairs to next floor */
    TILE_WATER    = 8,   /* Water (impassable, decorative) */
    TILE_CHEST    = 9,   /* Treasure chest */
    TILE_TRAP     = 10,  /* Trap (hidden) */
    TILE_TORCH    = 11,  /* Wall torch (light source) */
    TILE_ALTAR    = 12,  /* Boss room altar */
    TILE_BONES    = 13,  /* Decorative bones */
    TILE_MAX      = 14
};

/* ---- Tile attributes ---- */
typedef struct {
    u8 id;           /* Tile ID */
    u8 walkable;     /* Can the player walk here? */
    u8 transparent; /* Can you see through this tile? (for fog of war) */
    u8 wall;         /* Is this tile a wall? */
    u8 damage;       /* Does this tile deal damage? */
} TileAttr;

/* ---- Tilemap state ---- */
typedef struct {
    u8 tiles[MAP_HEIGHT][MAP_WIDTH];  /* Tile ID grid */
    u8 revealed[MAP_HEIGHT][MAP_WIDTH]; /* Fog of war: has player seen this? */
    u8 visible[MAP_HEIGHT][MAP_WIDTH];  /* Currently in player's line of sight */
} TileMap;

/* ---- Functions ---- */

/**
 * Initialize the tilemap system.
 * Clears all tiles to TILE_VOID.
 */
void tilemap_init(void);

/**
 * Get a pointer to the active tilemap.
 */
TileMap* tilemap_get(void);

/**
 * Set a tile at grid position (x, y).
 * @param x   Grid X (0..MAP_WIDTH-1)
 * @param y   Grid Y (0..MAP_HEIGHT-1)
 * @param id  Tile ID
 */
void tilemap_set_tile(u8 x, u8 y, u8 id);

/**
 * Get the tile ID at grid position (x, y).
 * @return Tile ID, or TILE_VOID if out of bounds.
 */
u8 tilemap_get_tile(u8 x, u8 y);

/**
 * Check if a tile is walkable.
 */
bool tilemap_is_walkable(u8 x, u8 y);

/**
 * Check if a tile is a wall.
 */
bool tilemap_is_wall(u8 x, u8 y);

/**
 * Get attributes for a tile type.
 */
TileAttr tilemap_get_attr(u8 tile_id);

/**
 * Reveal a tile (fog of war — mark as seen by player).
 */
void tilemap_reveal(u8 x, u8 y);

/**
 * Check if a tile has been revealed.
 */
bool tilemap_is_revealed(u8 x, u8 y);

/**
 * Mark a tile as currently visible (in player FOV this frame).
 */
void tilemap_set_visible(u8 x, u8 y, bool vis);

/**
 * Check if a tile is currently visible.
 */
bool tilemap_is_visible(u8 x, u8 y);

/**
 * Clear all visibility flags (call at start of each frame).
 */
void tilemap_clear_visibility(void);

/**
 * Reveal tiles around a position (simple radius-based FOV).
 * @param cx  Center X
 * @param cy  Center Y
 * @param radius  Visibility radius (in tiles, typically 5-8)
 */
void tilemap_reveal_around(u8 cx, u8 cy, u8 radius);

/**
 * Render the visible portion of the tilemap to BG layers.
 * Uses viewport culling to only process on-screen tiles.
 * Floor tiles go to BG0, wall overlay goes to BG1.
 */
void tilemap_render(void);

/**
 * Get the tile ID at screen coordinates (from touch or cursor).
 */
u8 tilemap_tile_at_screen(s16 sx, s16 sy);

#endif /* TILEMAP_H */
