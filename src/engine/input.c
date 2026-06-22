/**
 * input.c — GBA Keypad Input Implementation
 * 
 * Reads the hardware keypad register each frame and tracks
 * press/just-pressed/just-released states for all 10 keys.
 */

#include "gba_header.h"
#include "gba_types.h"
#include "input.h"

/* ---- Internal state ---- */
static u16 s_current = 0;      /* Keys currently held */
static u16 s_previous = 0;     /* Keys held last frame */
static u16 s_just_pressed = 0; /* Keys that went down this frame */
static u16 s_just_released = 0;/* Keys that went up this frame */

/* ---- Key repeat (for held-direction) ---- */
#define KEY_REPEAT_DELAY   15   /* Frames before repeat starts */
#define KEY_REPEAT_RATE    4    /* Frames between each repeat */
static u16 s_repeat_timers[10];
static u16 s_repeat_keys[10] = {
    KEY_A, KEY_B, KEY_SELECT, KEY_START,
    KEY_RIGHT, KEY_LEFT, KEY_UP, KEY_DOWN,
    KEY_R, KEY_L
};

/**
 * Initialize the input system.
 */
void input_init(void) {
    s_current = 0;
    s_previous = 0;
    s_just_pressed = 0;
    s_just_released = 0;
    
    for (int i = 0; i < 10; i++) {
        s_repeat_timers[i] = 0;
    }
}

/**
 * Read keypad hardware and update all state.
 */
void input_update(void) {
    s_previous = s_current;
    
    /* REG_KEYPAD is active-low: 0 = pressed, 1 = not pressed */
    s_current = ~REG_KEYPAD & 0x03FF;  /* Only lower 10 bits are keys */
    
    /* Compute edge detections */
    s_just_pressed  = s_current & ~s_previous;
    s_just_released = ~s_current & s_previous;
}

/**
 * Check if a key is currently held.
 */
bool key_pressed(u16 key) {
    return (s_current & key) ? TRUE : FALSE;
}

/**
 * Check if a key was just pressed this frame.
 */
bool key_just_pressed(u16 key) {
    return (s_just_pressed & key) ? TRUE : FALSE;
}

/**
 * Check if a key was just released this frame.
 */
bool key_just_released(u16 key) {
    return (s_just_released & key) ? TRUE : FALSE;
}

/**
 * Check if any key is pressed.
 */
bool key_any_pressed(void) {
    return s_current ? TRUE : FALSE;
}

/**
 * Block until at least one key is pressed.
 */
void input_wait_for_key(void) {
    while (!key_any_pressed()) {
        /* Busy wait — call input_update() if being polled */
    }
}

/**
 * Get raw key state.
 */
u16 input_get_raw(void) {
    return s_current;
}
