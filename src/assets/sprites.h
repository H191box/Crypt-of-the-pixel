/**
 * sprites.h — Sprite Graphics Data
 * 
 * Defines sprite graphics for player, enemies, items, and effects.
 * All sprites are 16×16 pixels, 4bpp format (128 bytes each).
 * 
 * Sprite layout in sprite VRAM:
 *   Tiles 0-3:   Player (4 directions, frame 0)
 *   Tiles 4-7:   Player (frame 1)
 *   Tiles 8-11:  Player (frame 2)
 *   Tiles 12-13: Slime (2 frames)
 *   Tiles 14-15: Skeleton (2 frames)
 *   Tiles 16-17: Bat (2 frames)
 *   Tiles 18-19: Ghost (2 frames)
 *   Tiles 20-21: Orc (2 frames)
 *   Tiles 22-23: Boss variants (2 frames)
 *   Tiles 24-25: Item sprites (potion, key, sword)
 *   Tiles 26-27: Effect sprites (explosion, spark)
 */

#ifndef SPRITES_H
#define SPRITES_H

#include "gba_types.h"

#define SPRITE_SIZE_BYTES   128  /* 16×16 pixels × 4bpp = 128 bytes */

/* ---- Player sprite data (4 directions × 3 frames) ---- */
extern const u8 spr_player_down_0[SPRITE_SIZE_BYTES];
extern const u8 spr_player_down_1[SPRITE_SIZE_BYTES];
extern const u8 spr_player_down_2[SPRITE_SIZE_BYTES];
extern const u8 spr_player_up_0[SPRITE_SIZE_BYTES];
extern const u8 spr_player_up_1[SPRITE_SIZE_BYTES];
extern const u8 spr_player_up_2[SPRITE_SIZE_BYTES];
extern const u8 spr_player_left_0[SPRITE_SIZE_BYTES];
extern const u8 spr_player_left_1[SPRITE_SIZE_BYTES];
extern const u8 spr_player_right_0[SPRITE_SIZE_BYTES];
extern const u8 spr_player_right_1[SPRITE_SIZE_BYTES];

/* ---- Enemy sprite data ---- */
extern const u8 spr_slime_0[SPRITE_SIZE_BYTES];
extern const u8 spr_slime_1[SPRITE_SIZE_BYTES];
extern const u8 spr_skeleton_0[SPRITE_SIZE_BYTES];
extern const u8 spr_skeleton_1[SPRITE_SIZE_BYTES];
extern const u8 spr_bat_0[SPRITE_SIZE_BYTES];
extern const u8 spr_bat_1[SPRITE_SIZE_BYTES];
extern const u8 spr_ghost_0[SPRITE_SIZE_BYTES];
extern const u8 spr_ghost_1[SPRITE_SIZE_BYTES];
extern const u8 spr_orc_0[SPRITE_SIZE_BYTES];
extern const u8 spr_orc_1[SPRITE_SIZE_BYTES];
extern const u8 spr_boss_slime_0[SPRITE_SIZE_BYTES];
extern const u8 spr_boss_slime_1[SPRITE_SIZE_BYTES];
extern const u8 spr_boss_skel_0[SPRITE_SIZE_BYTES];
extern const u8 spr_boss_demon_0[SPRITE_SIZE_BYTES];

/* ---- Item sprites ---- */
extern const u8 spr_potion[SPRITE_SIZE_BYTES];
extern const u8 spr_key[SPRITE_SIZE_BYTES];
extern const u8 spr_sword[SPRITE_SIZE_BYTES];

/* ---- Effect sprites ---- */
extern const u8 spr_explosion[SPRITE_SIZE_BYTES];
extern const u8 spr_spark[SPRITE_SIZE_BYTES];

/* ---- Load all sprites to sprite VRAM ---- */
void load_all_sprites(void);

#endif /* SPRITES_H */
