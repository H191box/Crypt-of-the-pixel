/**
 * iso_math.c — Isometric Coordinate Math Implementation
 * 
 * All coordinate conversions use integer arithmetic with the
 * camera offset applied. No floating-point needed — everything
 * is integer or fixed-point friendly.
 */

#include "gba_types.h"
#include "gba_header.h"
#include "iso_math.h"
#include "system.h"

/* ---- Camera instance ---- */
static IsoCamera s_camera;

/**
 * Convert grid (gx, gy) to screen pixel (sx, sy).
 * Formula: sx = (gx - gy) * HALF_W + SCREEN_CX + camera.x
 *          sy = (gx + gy) * HALF_H + SCREEN_CY + camera.y
 */
void iso_to_screen(s16 gx, s16 gy, s16* sx, s16* sy) {
    *sx = (s16)((gx - gy) * HALF_W) + SCREEN_CX + s_camera.x;
    *sy = (s16)((gx + gy) * HALF_H) + SCREEN_CY + s_camera.y;
}

/**
 * Convert screen to grid (inverse transform).
 * From: sx = (gx - gy) * HW + CX + cam.x
 *       sy = (gx + gy) * HH + CY + cam.y
 * 
 * Let rx = sx - CX - cam.x, ry = sy - CY - cam.y
 * Then: gx = (rx/HW + ry/HH) / 2
 *       gy = (ry/HH - rx/HW) / 2
 */
void screen_to_iso(s16 sx, s16 sy, s16* gx, s16* gy) {
    s16 rx = sx - SCREEN_CX - s_camera.x;
    s16 ry = sy - SCREEN_CY - s_camera.y;
    
    /* rx/HW = rx * 2 / TILE_W = rx / 8 (since HW=8) */
    /* ry/HH = ry * 2 / TILE_H = ry / 4 (since HH=4) */
    *gx = (rx / HALF_W + ry / HALF_H) / 2;
    *gy = (ry / HALF_H - rx / HALF_W) / 2;
}

/**
 * Convert grid + height to screen coords.
 * Used for drawing characters and wall tops above the floor.
 */
void iso_to_screen_height(s16 gx, s16 gy, s16 h, s16* sx, s16* sy) {
    iso_to_screen(gx, gy, sx, sy);
    *sy += h;  /* Negative h = drawn higher on screen */
}

/**
 * Initialize camera at the center of the map.
 */
void iso_camera_init(void) {
    /* Center on middle of map */
    s_camera.x = 0;
    s_camera.y = 0;
    s_camera.target_x = 0;
    s_camera.target_y = 0;
    
    /* Center on grid position (MAP_WIDTH/2, MAP_HEIGHT/2) */
    iso_camera_center(MAP_WIDTH / 2, MAP_HEIGHT / 2);
}

/**
 * Move camera by a pixel delta.
 */
void iso_move_camera(s16 dx, s16 dy) {
    s_camera.x += dx;
    s_camera.y += dy;
}

/**
 * Center camera on a grid position.
 */
void iso_camera_center(s16 gx, s16 gy) {
    /* We want grid (gx,gy) to appear at screen center.
     * iso_to_screen gives: sx = (gx-gy)*HW + CX + cam.x
     * We want sx = CX, so cam.x = -(gx-gy)*HW
     * Similarly: cam.y = -(gx+gy)*HH
     */
    s_camera.target_x = -(s16)((gx - gy) * HALF_W);
    s_camera.target_y = -(s16)((gx + gy) * HALF_H);
    
    /* Snap immediately (no smooth scroll on first center) */
    s_camera.x = s_camera.target_x;
    s_camera.y = s_camera.target_y;
}

/**
 * Smooth-scroll camera toward target.
 * Moves 1/4 of remaining distance each frame (lerp factor 0.25).
 */
void iso_camera_update(void) {
    s16 dx = s_camera.target_x - s_camera.x;
    s16 dy = s_camera.target_y - s_camera.y;
    
    /* Move 1/4 of remaining distance (integer math) */
    if (dx != 0) s_camera.x += dx >> 2;
    if (dy != 0) s_camera.y += dy >> 2;
    
    /* Snap if close enough */
    if (ABS(dx) < 2) s_camera.x = s_camera.target_x;
    if (ABS(dy) < 2) s_camera.y = s_camera.target_y;
}

/**
 * Get pointer to camera struct.
 */
IsoCamera* iso_get_camera(void) {
    return &s_camera;
}

/**
 * Get grid delta for a direction.
 */
void iso_dir_delta(u8 dir, s8* dx, s8* dy) {
    switch (dir) {
        case ISO_DIR_N: *dx =  0; *dy = -1; break;
        case ISO_DIR_S: *dx =  0; *dy =  1; break;
        case ISO_DIR_E: *dx =  1; *dy =  0; break;
        case ISO_DIR_W: *dx = -1; *dy =  0; break;
        default:        *dx =  0; *dy =  0; break;
    }
}

/**
 * Map D-pad to isometric grid delta.
 * 
 * In isometric view, the grid directions appear rotated 45°:
 *   UP    on screen => grid moves NW (gx--, gy--) — but for roguelike
 *                      we map UP to grid North (gy--) for simplicity
 *   DOWN  on screen => grid South (gy++)
 *   LEFT  on screen => grid West  (gx--)
 *   RIGHT on screen => grid East  (gx++)
 * 
 * This gives intuitive cardinal movement.
 */
void iso_dpad_to_delta(bool key_up, bool key_down, bool key_left, bool key_right,
                        s8* dx, s8* dy) {
    *dx = 0;
    *dy = 0;
    
    /* Priority: single direction takes full step, diagonal is single step too */
    if (key_up)    *dy = -1;
    if (key_down)  *dy =  1;
    if (key_left)  *dx = -1;
    if (key_right) *dx =  1;
    
    /* Cancel if both directions on same axis are pressed */
    if (key_up && key_down)  *dy = 0;
    if (key_left && key_right) *dx = 0;
}
