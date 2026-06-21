/**
 * dungeon_gen.h — Procedural Dungeon Generation
 * 
 * Generates random dungeon floors using BSP (Binary Space Partitioning).
 * Each floor is seeded, so the same seed always produces the same dungeon.
 * 
 * Room types:
 *   ROOM_NORMAL    — Standard room with 1-3 enemies
 *   ROOM_TREASURE  — Contains a treasure chest, no enemies
 *   ROOM_BOSS      — Boss room, always last room generated
 *   ROOM_SPAWN     — Player starting room, always first room
 *   ROOM_SECRET    — Hidden room behind a wall (optional)
 * 
 * Corridors connect rooms with 1-tile-wide hallways.
 */

#ifndef DUNGEON_GEN_H
#define DUNGEON_GEN_H

#include "gba_types.h"
#include "tilemap.h"

/* ---- Room types ---- */
typedef enum {
    ROOM_NORMAL    = 0,
    ROOM_TREASURE  = 1,
    ROOM_BOSS      = 2,
    ROOM_SPAWN     = 3,
    ROOM_SECRET    = 4,
    ROOM_MAX       = 5
} RoomType;

/* ---- Room structure ---- */
typedef struct {
    u8 x;          /* Top-left X in tile grid */
    u8 y;          /* Top-left Y in tile grid */
    u8 w;          /* Width in tiles (min 4, max 10) */
    u8 h;          /* Height in tiles (min 4, max 8) */
    u8 type;       /* RoomType */
    u8 cx;         /* Center X */
    u8 cy;         /* Center Y */
    bool connected; /* Has this room been connected to the network? */
} Room;

/* ---- Dungeon configuration ---- */
#define MAX_ROOMS           12
#define MIN_ROOM_SIZE       4
#define MAX_ROOM_WIDTH      10
#define MAX_ROOM_HEIGHT     8
#define BSP_MIN_SPLIT       20  /* Minimum area before stopping BSP split */
#define BSP_MAX_DEPTH       4   /* Maximum BSP recursion depth */

/* ---- Dungeon generation result ---- */
typedef struct {
    u32 seed;
    u8 floor_num;
    u8 room_count;
    Room rooms[MAX_ROOMS];
    u8 spawn_room;     /* Index of spawn room */
    u8 boss_room;      /* Index of boss room */
    u8 stair_x;        /* Stairs to next floor position */
    u8 stair_y;
} DungeonData;

/* ---- Functions ---- */

/**
 * Generate a new dungeon floor.
 * @param seed       Random seed (same seed = same dungeon)
 * @param floor_num  Current floor number (affects difficulty/enemy types)
 * @return Pointer to generated dungeon data (valid until next generate call).
 */
DungeonData* dungeon_generate(u32 seed, u8 floor_num);

/**
 * Get the current dungeon data.
 */
DungeonData* dungeon_get(void);

/**
 * Place all generated rooms onto the tilemap.
 * Fills floor tiles, walls, doors, stairs, etc.
 */
void dungeon_place_tiles(void);

/**
 * Get the spawn position for the player.
 */
void dungeon_get_spawn(u8* x, u8* y);

/**
 * Check if position (x,y) is inside any room.
 * @return Room index, or -1 if not in any room.
 */
s8 dungeon_room_at(u8 x, u8 y);

/**
 * Get the nearest room center to position (x,y).
 * @return Room index.
 */
s8 dungeon_nearest_room(u8 x, u8 y);

#endif /* DUNGEON_GEN_H */
