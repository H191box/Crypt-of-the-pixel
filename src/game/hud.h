/**
 * hud.h — Heads-Up Display
 * 
 * Renders game UI elements:
 * - HP/MP bars (top-left)
 * - Floor number and level
 * - Mini-map (top-right, small overview)
 * - Message log (bottom, scrolling combat/pickup messages)
 * - Status indicators
 * 
 * Uses BG3 (text layer) for all HUD rendering.
 */

#ifndef HUD_H
#define HUD_H

#include "gba_types.h"

/* ---- HUD layout constants ---- */
#define HUD_BAR_X         1     /* HP bar tile X */
#define HUD_BAR_Y         1     /* HP bar tile Y */
#define HUD_BAR_WIDTH     10    /* HP bar width in tiles */
#define HUD_BAR_HEIGHT    1     /* HP bar height */

#define HUD_FLOOR_X       1
#define HUD_FLOOR_Y       3

#define HUD_MINIMAP_X     22    /* Mini-map tile X (right side) */
#define HUD_MINIMAP_Y     1     /* Mini-map tile Y */
#define HUD_MINIMAP_SIZE  8     /* Mini-map size in tiles (64×64 pixel = 8×8 tile) */

#define HUD_MSG_X         1     /* Message log X */
#define HUD_MSG_Y         16    /* Message log Y (near bottom) */
#define HUD_MSG_LINES      4    /* Visible message lines */
#define HUD_MSG_WIDTH     30    /* Characters per line */

#define HUD_GOLD_X        12
#define HUD_GOLD_Y        1

/* ---- Message log ---- */
#define MSG_LOG_SIZE     32
#define MSG_MAX_LEN      40

/* ---- Functions ---- */

/**
 * Initialize the HUD system.
 * Sets up the text layer and draws initial HUD frame.
 */
void hud_init(void);

/**
 * Update and redraw the HUD.
 * Call once per frame after game state updates.
 */
void hud_update(void);

/**
 * Draw HP and MP bars.
 */
void hud_draw_bars(void);

/**
 * Draw floor number and player level.
 */
void hud_draw_info(void);

/**
 * Draw the mini-map.
 * Shows revealed tiles as tiny dots. Player as a blinking dot.
 */
void hud_draw_minimap(void);

/**
 * Draw the message log.
 */
void hud_draw_messages(void);

/**
 * Add a message to the log.
 * @param str  Message string (max MSG_MAX_LEN chars).
 */
void hud_add_message(const char* str);

/**
 * Clear all messages.
 */
void hud_clear_messages(void);

/**
 * Draw a temporary overlay (e.g., "LEVEL UP!" or "GAME OVER").
 * @param text  Overlay text.
 * @param timer Frames to display (0 = until cleared).
 */
void hud_show_overlay(const char* text, u16 timer);

/**
 * Clear the overlay.
 */
void hud_clear_overlay(void);

#endif /* HUD_H */
