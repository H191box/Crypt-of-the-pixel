/**
 * iso_math.h — Isometric Coordinate Mathematics
 * 
 * Converts between grid coordinates (gx, gy) and screen
 * coordinates (sx, sy) for diamond-tile isometric rendering.
 * 
 * Our isometric projection:
 *   TILE_W = 16 pixels wide (half-width = 8)
 *   TILE_H = 8 pixels tall  (half-height = 4)
 * 
 * Grid-to-screen:
 *   sx = (gx - gy) * HALF_W + camera_x
 *   sy = (gx + gy) * HALF_H + camera_y
 * 
 * Screen-to-grid (inverse):
 *   gx = (sx / HALF_W + sy / HALF_H) / 2
 *   gy = (sy / HALF_H - sx / HALF_W) / 2
 */

#ifndef ISO_MATH_H
#define ISO_MATH_H

#include "gba_types.h"

/* ---- Tile dimensions (isometric diamond) ---- */
#define TILE_W          16    /* Full width of isometric tile */
#define TILE_H          8     /* Full height of isometric tile */
#define HALF_W          8     /* TILE_W / 2 */
#define HALF_H          4     /* TILE_H / 2 */

/* ---- Map dimensions ---- */
#define MAP_WIDTH       64    /* Tiles wide */
#define MAP_HEIGHT      64    /* Tiles tall */

/* ---- Screen center offset ---- */
/* The isometric map origin (0,0) renders at screen center so the map
 * expands outward in all four directions as the player explores. */
#define SCREEN_CX      (SCREEN_WIDTH / 2)
#define SCREEN_CY      (SCREEN_HEIGHT / 2)

/* ---- Camera ---- */
typedef struct {
    s16 x;           /* Camera offset X (screen pixels) */
    s16 y;           /* Camera offset Y (screen pixels) */
    s16 target_x;    /* Target X (for smooth scrolling) */
    s16 target_y;    /* Target Y */
} IsoCamera;

/* ---- Coordinate conversion ---- */

/**
 * Convert grid (tile) coordinates to screen pixel coordinates.
 * @param gx  Grid X coordinate (0..MAP_WIDTH-1)
 * @param gy  Grid Y coordinate (0..MAP_HEIGHT-1)
 * @param sx  Output screen X (0..SCREEN_WIDTH-1)
 * @param sy  Output screen Y (0..SCREEN_HEIGHT-1)
 */
void iso_to_screen(s16 gx, s16 gy, s16* sx, s16* sy);

/**
 * Convert screen pixel coordinates to grid (tile) coordinates.
 * Inverse of iso_to_screen.
 * @param sx  Screen X
 * @param sy  Screen Y
 * @param gx  Output grid X
 * @param gy  Output grid Y
 */
void screen_to_iso(s16 sx, s16 sy, s16* gx, s16* gy);

/**
 * Convert grid + height offset to screen coords (for walls/characters).
 * @param gx  Grid X
 * @param gy  Grid Y
 * @param h   Height offset in pixels (negative = up)
 * @param sx  Output screen X
 * @param sy  Output screen Y
 */
void iso_to_screen_height(s16 gx, s16 gy, s16 h, s16* sx, s16* sy);

/* ---- Camera ---- */

/**
 * Initialize the camera to center of the map.
 */
void iso_camera_init(void);

/**
 * Move camera by a delta (pixels).
 * @param dx  Pixel offset X
 * @param dy  Pixel offset Y
 */
void iso_move_camera(s16 dx, s16 dy);

/**
 * Center the camera on a grid position.
 * @param gx  Grid X to center on
 * @param gy  Grid Y to center on
 */
void iso_camera_center(s16 gx, s16 gy);

/**
 * Smooth-scroll camera toward its target position.
 * Call once per frame.
 */
void iso_camera_update(void);

/**
 * Get current camera position.
 */
IsoCamera* iso_get_camera(void);

/* ---- Isometric direction helpers ---- */

/**
 * The 4 isometric directions: N, S, E, W on the grid map to
 * their corresponding screen-space offsets.
 * 
 * In our isometric view:
 *   N (gy--) => screen moves up-right
 *   S (gy++) => screen moves down-left  
 *   E (gx++) => screen moves down-right
 *   W (gx--) => screen moves up-left
 */
enum IsoDirection {
    ISO_DIR_N = 0,   /* North: gy-- */
    ISO_DIR_S = 1,   /* South: gy++ */
    ISO_DIR_E = 2,   /* East:  gx++ */
    ISO_DIR_W = 3    /* West:  gx-- */
};

/**
 * Get grid delta for an isometric direction.
 * @param dir  IsoDirection value
 * @param dx   Output delta X
 * @param dy   Output delta Y
 */
void iso_dir_delta(u8 dir, s8* dx, s8* dy);

/**
 * Map D-pad keys to isometric directions.
 * @param key_up    TRUE if UP is pressed
 * @param key_down  TRUE if DOWN is pressed
 * @param key_left  TRUE if LEFT is pressed
 * @param key_right TRUE if RIGHT is pressed
 * @param dx        Output movement delta X
 * @param dy        Output movement delta Y
 */
void iso_dpad_to_delta(bool key_up, bool key_down, bool key_left, bool key_right, 
                        s8* dx, s8* dy);

#endif /* ISO_MATH_H */
