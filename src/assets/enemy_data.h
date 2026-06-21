/**
 * enemy_data.h — Enemy Type Definitions
 * 
 * Table of all enemy types with their base stats, behavior,
 * and visual properties. Floor scaling is applied at spawn time.
 */

#ifndef ENEMY_DATA_H
#define ENEMY_DATA_H

#include "gba_types.h"

/* ---- Enemy definition ---- */
typedef struct {
    u8 id;             /* EnemyType enum value */
    const char* name;  /* Enemy name (max 12 chars) */
    s16 base_hp;       /* Base HP */
    u8 base_atk;       /* Base attack */
    u8 base_def;       /* Base defense */
    u8 base_speed;     /* Tiles per turn (1=every turn, 2=every other) */
    u8 xp_reward;      /* Base XP on kill */
    u8 gold_reward;    /* Base gold on kill */
    u8 sprite_tile;    /* Base sprite tile index */
    u8 min_floor;      /* Minimum floor this enemy appears on */
    u8 behavior;       /* 0=patrol, 1=chase, 2=stationary */
} EnemyDef;

/* ---- Enemy definition table ---- */
extern const EnemyDef g_enemy_defs[];
extern const u8 g_enemy_def_count;

#endif /* ENEMY_DATA_H */
