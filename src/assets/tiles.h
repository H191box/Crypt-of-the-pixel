/**
 * tiles.h — Isometric Tile Graphics Data
 * 
 * Each isometric floor tile is 16×8 pixels (diamond shape).
 * Stored as 4bpp (4 bits per pixel = 16 colors per palette).
 * Each 8×8 GBA tile = 32 bytes, so each iso tile = 64 bytes (2 GBA tiles).
 * 
 * Tiles are stored as pairs: left half (8×8) and right half (8×8).
 * The diamond shape is achieved by making the left/right edges transparent.
 * 
 * Tile layout in VRAM:
 *   Tile 0-1: Floor stone
 *   Tile 2-3: Floor stone variant 2
 *   Tile 4-5: Floor cracked
 *   Tile 6-7: Wall (brick pattern)
 *   ... etc.
 * Dark variants offset by TILE_MAX*2.
 */

#ifndef TILES_H
#define TILES_H

#include "gba_types.h"

/* ---- Tile data sizes ---- */
#define ISO_TILE_BYTES    64   /* 2 GBA tiles × 32 bytes each */
#define NUM_TILE_TYPES    14   /* Matches TILE_MAX in tilemap.h */
#define TOTAL_TILE_BYTES  (NUM_TILE_TYPES * ISO_TILE_BYTES * 2)  /* Normal + dark */
#define GBA_TILE_BYTES    32   /* One 8×8 4bpp tile */

/* ---- Tile data arrays ---- */
/* Each tile type has 2 GBA tiles (left and right halves of the diamond) */
extern const u8 tile_data_floor1[];       /* 64 bytes */
extern const u8 tile_data_floor2[];       /* 64 bytes */
extern const u8 tile_data_floor3[];       /* 64 bytes */
extern const u8 tile_data_wall[];          /* 64 bytes */
extern const u8 tile_data_wall2[];         /* 64 bytes */
extern const u8 tile_data_door[];          /* 64 bytes */
extern const u8 tile_data_stairs[];        /* 64 bytes */
extern const u8 tile_data_water[];         /* 64 bytes */
extern const u8 tile_data_chest[];         /* 64 bytes */
extern const u8 tile_data_trap[];          /* 64 bytes */
extern const u8 tile_data_torch[];         /* 64 bytes */
extern const u8 tile_data_altar[];         /* 64 bytes */
extern const u8 tile_data_bones[];         /* 64 bytes */
extern const u8 tile_data_void[];          /* 64 bytes */

/* ---- Load all tile graphics to VRAM ---- */
void load_all_tiles(void);

#endif /* TILES_H */
