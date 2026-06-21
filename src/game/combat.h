/**
 * combat.h — Turn-Based Combat System
 * 
 * When the player is adjacent to an enemy and attacks (or an enemy
 * attacks the player), combat is resolved turn-by-turn.
 * 
 * Combat flow:
 * 1. Player selects action: Attack, Magic, Item, Flee
 * 2. Player action resolves
 * 3. Enemy action resolves
 * 4. Check for death/victory
 * 5. Repeat until one side is defeated or player flees
 */

#ifndef COMBAT_H
#define COMBAT_H

#include "gba_types.h"

/* ---- Combat states ---- */
typedef enum {
    COMBAT_IDLE       = 0,  /* No combat in progress */
    COMBAT_PLAYER_TURN = 1, /* Waiting for player input */
    COMBAT_ENEMY_TURN  = 2, /* Enemy is acting */
    COMBAT_RESULT      = 3, /* Showing combat result */
    COMBAT_FLED        = 4  /* Player fled */
} CombatState;

/* ---- Combat actions ---- */
typedef enum {
    ACTION_ATTACK = 0,
    ACTION_MAGIC  = 1,
    ACTION_ITEM   = 2,
    ACTION_FLEE   = 3
} CombatAction;

/* ---- Combat result data ---- */
typedef struct {
    s16 player_dmg_dealt;    /* Damage player dealt this round */
    s16 player_dmg_taken;    /* Damage player took this round */
    bool enemy_killed;       /* Did the enemy die? */
    bool player_dodged;      /* Did the player dodge? */
    bool player_crit;        /* Was it a critical hit? */
    bool flee_success;       /* Did flee succeed? */
} CombatResult;

/* ---- Functions ---- */

/**
 * Initialize combat system.
 */
void combat_init(void);

/**
 * Start combat with a specific enemy.
 * @param enemy_id  Enemy index to fight.
 */
void combat_start(s8 enemy_id);

/**
 * End combat (return to dungeon mode).
 */
void combat_end(void);

/**
 * Get current combat state.
 */
CombatState combat_get_state(void);

/**
 * Get the current target enemy ID.
 */
s8 combat_get_target(void);

/**
 * Get last combat result.
 */
CombatResult combat_get_result(void);

/**
 * Process player's chosen action.
 * @param action  What the player wants to do.
 */
void combat_player_action(CombatAction action);

/**
 * Process enemy's action (AI).
 */
void combat_enemy_action(void);

/**
 * Check if combat is in progress.
 */
bool combat_active(void);

#endif /* COMBAT_H */
