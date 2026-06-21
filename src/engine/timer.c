/**
 * timer.c — GBA Timer Implementation
 * 
 * Uses Timer 0 as a free-running millisecond timer.
 * Provides frame counting and delta time for game logic.
 */

#include "gba_header.h"
#include "gba_types.h"
#include "timer.h"
#include "system.h"

/* ---- Internal state ---- */
static u32 s_frames = 0;
static u32 s_last_frames = 0;
static u32 s_delta = 0;

/**
 * Initialize timer subsystem.
 */
void timer_init(void) {
    s_frames = 0;
    s_last_frames = 0;
    s_delta = 0;
    
    /* 
     * Timer 0: free-running at 1024 cycles/tick.
     * At 16.78 MHz, that's ~16.38 kHz, or ~61 microseconds per tick.
     * Timer overflows every 65536 ticks = ~4 seconds.
     * We cascade Timer 1 off Timer 0 for longer timing if needed.
     */
    
    /* Stop all timers first */
    REG_TM0CNT = 0;
    REG_TM1CNT = 0;
    REG_TM2CNT = 0;
    REG_TM3CNT = 0;
}

/**
 * Update frame count. Call this once per frame.
 */
void timer_update(void) {
    s_last_frames = s_frames;
    s_frames = g_frame_count;
    s_delta = s_frames - s_last_frames;
    if (s_delta > 10) s_delta = 1; /* Clamp delta for safety */
}

/**
 * Get total frames elapsed.
 */
u32 timer_get_frames(void) {
    return s_frames;
}

/**
 * Get delta frames since last timer_update().
 */
u32 timer_get_delta(void) {
    return s_delta;
}

/**
 * Start a specific timer.
 */
void timer_start(u8 timer_id, u16 frequency) {
    vu16* cnt_regs[4] = {
        &REG_TM0CNT, &REG_TM1CNT, &REG_TM2CNT, &REG_TM3CNT
    };
    vu16* data_regs[4] = {
        &REG_TM0D, &REG_TM1D, &REG_TM2D, &REG_TM3D
    };
    
    if (timer_id > 3) return;
    
    /* Reset counter */
    *data_regs[timer_id] = 0;
    
    /* Start with prescaler, enable, no IRQ */
    *cnt_regs[timer_id] = (frequency & 0x03) | TM_ENABLE;
}

/**
 * Stop a timer.
 */
void timer_stop(u8 timer_id) {
    vu16* cnt_regs[4] = {
        &REG_TM0CNT, &REG_TM1CNT, &REG_TM2CNT, &REG_TM3CNT
    };
    if (timer_id > 3) return;
    *cnt_regs[timer_id] = 0;
}

/**
 * Read a timer's current counter value.
 */
u16 timer_read(u8 timer_id) {
    vu16* data_regs[4] = {
        &REG_TM0D, &REG_TM1D, &REG_TM2D, &REG_TM3D
    };
    if (timer_id > 3) return 0;
    return *data_regs[timer_id];
}

/**
 * Reset a timer's counter to 0.
 */
void timer_reset(u8 timer_id) {
    vu16* data_regs[4] = {
        &REG_TM0D, &REG_TM1D, &REG_TM2D, &REG_TM3D
    };
    if (timer_id > 3) return;
    *data_regs[timer_id] = 0;
}

/**
 * Countdown timer helper.
 * Returns TRUE when 'frames' frames have elapsed.
 */
bool timer_countdown(u16 frames) {
    static u32 countdown_start = 0;
    
    if (countdown_start == 0) {
        countdown_start = g_frame_count;
        return FALSE;
    }
    
    if (g_frame_count - countdown_start >= frames) {
        countdown_start = 0;
        return TRUE;
    }
    return FALSE;
}
