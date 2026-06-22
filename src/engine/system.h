/**
 * system.h — GBA System Initialization and Utilities
 * 
 * Handles hardware setup: video mode, background layers,
 * sprite engine, VBlank interrupts, and frame counting.
 */

#ifndef SYSTEM_H
#define SYSTEM_H

#include "gba_types.h"
#include "gba_header.h"  /* MODE_0, etc. */

/* ---- Display mode we use ---- */
#define SCREEN_MODE   MODE_0    /* 4 text BG layers for tile-based isometric */

/* ---- BG layer assignments ---- */
/* BG0: Main isometric floor tiles    (priority 0 — behind everything) */
/* BG1: Wall/obstacle overlay tiles   (priority 1)                     */
/* BG2: Mini-map                       (priority 2)                     */
/* BG3: HUD text layer                 (priority 3 — in front)          */

#define BG_FLOOR_LAYER    0
#define BG_WALL_LAYER     1
#define BG_MINIMAP_LAYER  2
#define BG_HUD_LAYER      3

/* ---- Screen dimensions ---- */
#define SCREEN_WIDTH    240
#define SCREEN_HEIGHT   160

/* ---- Sprite dimensions for OAM ---- */
#define SPRITE_8x8      0
#define SPRITE_16x16    1
#define SPRITE_32x32    2
#define SPRITE_64x64    3

/* ---- VBlank callback type ---- */
typedef void (*VBlankCallback)(void);

/* ---- Functions ---- */

/**
 * Initialize GBA hardware.
 * Sets video mode, background layers, OAM, palettes, and interrupts.
 * MUST be called first thing in main().
 */
void sys_init(void);

/**
 * Wait for VBlank (vertical blanking period).
 * Blocks until the current frame finishes rendering.
 * Call at the end of the game loop.
 */
void sys_wait_vblank(void);

/**
 * Register a callback to be called during each VBlank interrupt.
 * @param cb  Function pointer (may be NULL to unregister).
 */
void sys_set_vblank_callback(VBlankCallback cb);

/**
 * Get the current frame counter (increments each VBlank).
 * @return Frame number since sys_init() was called.
 */
u32 sys_get_frame_count(void);

/**
 * Get the current VBlank count (same as frame count).
 */
u32 sys_get_vblank_count(void);

/**
 * Simple busy-wait for a number of cycles.
 * Not precise — use timers for accurate timing.
 */
void sys_delay_cycles(u32 cycles);

/**
 * Halt the CPU until next interrupt.
 */
void sys_halt(void);

/**
 * Soft reset — jump back to ROM entry point.
 * Not a real hardware reset but reboots the game.
 */
void NORETURN sys_reset(void);

/* ---- Global state ---- */
extern u32 g_frame_count;

#endif /* SYSTEM_H */
