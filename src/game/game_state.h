/**
 * game_state.h — Game State Machine
 * 
 * Manages the top-level game states and transitions.
 * Handles game initialization, updates, rendering, and save/load.
 * 
 * States:
 *   TITLE    — Title screen with "Press START"
 *   CLASS_SEL — Character class selection
 *   DUNGEON  — Main dungeon exploration (movement, items)
 *   COMBAT   — Combat mode (attack/magic/item/flee)
 *   INVENTORY — Inventory management screen
 *   GAME_OVER — Death screen
 *   VICTORY  — Win screen (cleared all floors)
 */

#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "gba_types.h"

/* ---- Game states ---- */
typedef enum {
    STATE_TITLE     = 0,
    STATE_CLASS_SEL = 1,
    STATE_DUNGEON   = 2,
    STATE_COMBAT    = 3,
    STATE_INVENTORY = 4,
    STATE_GAME_OVER = 5,
    STATE_VICTORY   = 6,
    STATE_SAVE_LOAD = 7
} GameState;

/* ---- Game constants ---- */
#define MAX_FLOOR       10       /* Total dungeon floors */
#define SAVE_MAGIC      0x4354    /* "CT" for Crypt */
#define SAVE_VERSION    1

/* ---- Save data structure ---- */
typedef struct {
    u16 magic;        /* Must be SAVE_MAGIC */
    u8  version;      /* Save format version */
    u32 dungeon_seed;
    u8  floor_num;
    u8  player_class;
    u16 player_hp;
    u16 player_max_hp;
    u16 player_mp;
    u16 player_max_mp;
    u8  player_level;
    u16 player_xp;
    u16 player_gold;
    u8  player_gx;
    u8  player_gy;
    u16 checksum;      /* Simple XOR checksum */
} SaveData;

/* ---- Functions ---- */

/**
 * Initialize the entire game.
 * Called once at startup.
 */
void game_init(void);

/**
 * Main game update — called once per frame.
 * Processes input, updates current state, advances game logic.
 */
void game_update(void);

/**
 * Main game render — called once per frame after update.
 * Draws the current state to screen.
 */
void game_render(void);

/**
 * Change the game state.
 * @param new_state  State to transition to.
 */
void game_change_state(GameState new_state);

/**
 * Get current game state.
 */
GameState game_get_state(void);

/**
 * Save game to SRAM.
 * @return TRUE if save successful.
 */
bool game_save(void);

/**
 * Load game from SRAM.
 * @return TRUE if valid save data found and loaded.
 */
bool game_load(void);

/**
 * Check if a save file exists.
 */
bool game_has_save(void);

/**
 * Start a new game with the given player class.
 */
void game_new_game(u8 player_class);

/**
 * Advance to the next dungeon floor.
 */
void game_next_floor(void);

#endif /* GAME_STATE_H */
