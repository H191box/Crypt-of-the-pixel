/**
 * timer.h — GBA Timer Utilities
 * 
 * The GBA has 4 16-bit timers with cascading capability.
 * Used for frame counting, delta time, and general timing.
 */

#ifndef TIMER_H
#define TIMER_H

#include "gba_types.h"

/* Timer IDs */
#define TIMER_0   0
#define TIMER_1   1
#define TIMER_2   2
#define TIMER_3   3

/* ---- Functions ---- */

/**
 * Initialize the timer subsystem.
 * Sets up Timer 0 for general-purpose frame timing.
 */
void timer_init(void);

/**
 * Get the number of VBlank frames since timer_init().
 */
u32 timer_get_frames(void);

/**
 * Get delta time in frames since last call to timer_get_delta().
 * Useful for frame-rate independent logic.
 */
u32 timer_get_delta(void);

/**
 * Start a timer at a given frequency.
 * @param timer_id   Timer number (0-3)
 * @param frequency  Prescaler: TM_FREQ_1, TM_FREQ_64, TM_FREQ_256, TM_FREQ_1024
 */
void timer_start(u8 timer_id, u16 frequency);

/**
 * Stop a timer.
 * @param timer_id  Timer number (0-3)
 */
void timer_stop(u8 timer_id);

/**
 * Read the current value of a timer's counter.
 * @param timer_id  Timer number (0-3)
 * @return Current 16-bit counter value.
 */
u16 timer_read(u8 timer_id);

/**
 * Reset a timer's counter to 0 (without stopping).
 */
void timer_reset(u8 timer_id);

/**
 * Simple frame-based countdown timer.
 * @param frames  Number of frames to count down.
 * @return TRUE when countdown reaches 0.
 */
bool timer_countdown(u16 frames);

#endif /* TIMER_H */
