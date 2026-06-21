/**
 * input.h — GBA Keypad Input Handler
 * 
 * Provides debounced key state tracking with:
 * - Current pressed state
 * - Just pressed (edge detection on press)
 * - Just released (edge detection on release)
 */

#ifndef INPUT_H
#define INPUT_H

#include "gba_types.h"

/* ---- Key constants (re-export for convenience) ---- */
#define KEY_A           (1 << 0)
#define KEY_B           (1 << 1)
#define KEY_SELECT      (1 << 2)
#define KEY_START       (1 << 3)
#define KEY_RIGHT       (1 << 4)
#define KEY_LEFT        (1 << 5)
#define KEY_UP          (1 << 6)
#define KEY_DOWN        (1 << 7)
#define KEY_R           (1 << 8)
#define KEY_L           (1 << 9)

/* Directional keys combined */
#define KEY_DIR         (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)

/* Action keys combined */
#define KEY_ACTION      (KEY_A | KEY_B | KEY_START | KEY_SELECT)

/* ---- Functions ---- */

/**
 * Initialize the input system. 
 * Call once during startup. Zeros all key states.
 */
void input_init(void);

/**
 * Update the input state.
 * Call once per frame (ideally at the start of the game loop).
 * Reads hardware register and computes just_pressed/released.
 */
void input_update(void);

/**
 * Check if a key is currently held down.
 * @param key  One of KEY_A, KEY_B, KEY_UP, etc. (or combination)
 * @return TRUE if the key is pressed this frame.
 */
bool key_pressed(u16 key);

/**
 * Check if a key was just pressed this frame (rising edge).
 * Useful for menu navigation and one-shot actions.
 * @param key  One of KEY_A, KEY_B, KEY_UP, etc.
 * @return TRUE if the key was pressed since the last input_update().
 */
bool key_just_pressed(u16 key);

/**
 * Check if a key was just released this frame (falling edge).
 * @param key  One of KEY_A, KEY_B, KEY_UP, etc.
 * @return TRUE if the key was released since the last input_update().
 */
bool key_just_released(u16 key);

/**
 * Check if ANY key is pressed.
 * @return TRUE if at least one key is currently held.
 */
bool key_any_pressed(void);

/**
 * Block until at least one key is pressed (useful for title screen).
 */
void input_wait_for_key(void);

/**
 * Get raw key state (directly from REG_KEYPAD, inverted).
 */
u16 input_get_raw(void);

#endif /* INPUT_H */
