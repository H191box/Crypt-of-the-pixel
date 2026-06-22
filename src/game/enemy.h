/**
 * enemy.h — Enemy System
 * 
 * Manages all enemies on the current dungeon floor.
 * Enemies have simple AI: move toward player if within detection
 * range, otherwise random patrol. Turn-based: enemies act after
 * the player's turn.
 */

#ifndef ENEMY_H
#define ENEMY_H

#include "gba_types.h"

/* ---- Enemy type IDs ---- */
typedef enum {
    ENEMY_SLIME     = 0,
    ENEMY_BAT       = 1,
    ENEMY_SKELETON  = 2,
    ENEMY_GHOST     = 3,
    ENEMY_ORC       = 4,
    ENEMY_BOSS_SLIME   = 10,
    ENEMY_BOSS_SKELE   = 11,
    ENEMY_BOSS_DEMON   = 12,
    ENEMY_MAX = 13
} EnemyType;

/* ---- Enemy state ---- */
typedef struct {
    bool active;       /* Is this enemy alive and on the map? */
    EnemyType type;    /* Enemy type */
    u8 gx;             /* Grid X position */
    u8 gy;             /* Grid Y position */
    s16 hp;            /* Current HP */
    s16 max_hp;        /* Max HP */
    u8 attack;         /* Attack power */
    u8 defense;        /* Defense */
    u8 speed;          /* Movement speed (tiles per turn) */
    u8 xp_reward;     /* XP given when killed */
    u8 gold_reward;    /* Gold given when killed */
    u8 sprite_id;      /* Sprite tile index */
    u8 anim_frame;     /* Current animation frame */
    u8 move_timer;     /* Turn cooldown (some enemies move every other turn) */
    u8 turn_count;     /* Turns alive */
} Enemy;

/* ---- Constants ---- */
#define MAX_ENEMIES     32
#define DETECT_RANGE    8   /* Enemy detects player within this tile distance */

/* ---- Functions ---- */

/**
 * Initialize the enemy system. Clears all enemies.
 */
void enemy_init(void);

/**
 * Create an enemy at grid position.
 * @param type  Enemy type
 * @param gx    Grid X
 * @param gy    Grid Y
 * @return Enemy index, or -1 if max reached.
 */
s8 enemy_create(EnemyType type, u8 gx, u8 gy);

/**
 * Remove an enemy (death or off-screen).
 */
void enemy_remove(s8 id);

/**
 * Get pointer to an enemy.
 */
Enemy* enemy_get(s8 id);

/**
 * Update all active enemies (AI turn).
 * Call after the player has taken their action.
 */
void enemy_update_all(void);

/**
 * Damage an enemy.
 * @param id      Enemy index
 * @param amount  Damage amount
 * @return HP remaining, or -1 if enemy died.
 */
s16 enemy_damage(s8 id, s16 amount);

/**
 * Check if any enemy is at grid position (x,y).
 * @return Enemy index, or -1 if no enemy there.
 */
s8 enemy_at(u8 x, u8 y);

/**
 * Get count of active enemies.
 */
u8 enemy_count(void);

/**
 * Place enemies in all dungeon rooms (except spawn and boss).
 * Number and types scale with floor number.
 */
void enemy_populate_dungeon(void);

/**
 * Get the nearest alive enemy to position (x,y).
 * @return Enemy index, or -1 if none.
 */
s8 enemy_nearest(u8 x, u8 y);

#endif /* ENEMY_H */
