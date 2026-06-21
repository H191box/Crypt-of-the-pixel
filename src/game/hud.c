/**
 * hud.c — HUD Implementation
 * 
 * Renders all UI elements on BG3 (text layer).
 * HP/MP bars use filled blocks (custom tiles).
 * Messages scroll upward as new ones arrive.
 */

#include "gba_types.h"
#include "gba_header.h"
#include "player.h"
#include "dungeon_gen.h"
#include "tilemap.h"
#include "text_renderer.h"
#include "iso_math.h"
#include "hud.h"

/* ---- Message log ---- */
static char s_msg_log[MSG_LOG_SIZE][MSG_MAX_LEN];
static u8 s_msg_head;      /* Next message slot to write */
static u8 s_msg_count;     /* Total messages */

/* ---- Overlay ---- */
static char s_overlay_text[20];
static u16 s_overlay_timer;

/**
 * Initialize HUD.
 */
void hud_init(void) {
    s_msg_head = 0;
    s_msg_count = 0;
    s_overlay_timer = 0;
    s_overlay_text[0] = '\0';
    
    /* Draw HUD border/frame */
    hud_add_message("Welcome to the Crypt!");
    hud_add_message("Use D-pad to move.");
}

/**
 * Update HUD — called each frame.
 */
void hud_update(void) {
    /* Update overlay timer */
    if (s_overlay_timer > 0) {
        s_overlay_timer--;
        if (s_overlay_timer == 0) {
            s_overlay_text[0] = '\0';
        }
    }
    
    /* Clear text layer */
    text_clear_all();
    
    /* Draw all HUD elements */
    hud_draw_bars();
    hud_draw_info();
    hud_draw_minimap();
    hud_draw_messages();
    
    /* Draw overlay if active */
    if (s_overlay_timer > 0) {
        text_draw_px(80, 60, s_overlay_text, TEXT_COLOR_YELLOW);
    }
    
    /* Flush text to VRAM */
    text_flush();
}

/**
 * Draw HP and MP bars.
 * Uses block characters to create filled bars.
 */
void hud_draw_bars(void) {
    Player* pl = player_get();
    
    /* HP Bar */
    text_draw(HUD_BAR_X, HUD_BAR_Y, "HP", TEXT_COLOR_RED);
    
    /* Calculate filled portion */
    u8 hp_pct = 0;
    if (pl->max_hp > 0) {
        hp_pct = (u8)((u32)pl->hp * 10 / (u32)pl->max_hp);
    }
    
    /* Draw bar as filled blocks */
    char bar_buf[12];
    int bi = 0;
    for (int i = 0; i < 10; i++) {
        bar_buf[bi++] = (i < hp_pct) ? '#' : '.';
    }
    bar_buf[bi] = '\0';
    text_draw(HUD_BAR_X + 3, HUD_BAR_Y, bar_buf, TEXT_COLOR_RED);
    
    /* HP numbers */
    text_draw_int_px((HUD_BAR_X + 3) * 8 + 80, HUD_BAR_Y * 8, pl->hp, TEXT_COLOR_WHITE);
    
    /* MP Bar */
    text_draw(HUD_BAR_X, HUD_BAR_Y + 1, "MP", TEXT_COLOR_BLUE);
    
    u8 mp_pct = 0;
    if (pl->max_mp > 0) {
        mp_pct = (u8)((u32)pl->mp * 10 / (u32)pl->max_mp);
    }
    
    bi = 0;
    for (int i = 0; i < 10; i++) {
        bar_buf[bi++] = (i < mp_pct) ? '#' : '.';
    }
    bar_buf[bi] = '\0';
    text_draw(HUD_BAR_X + 3, HUD_BAR_Y + 1, bar_buf, TEXT_COLOR_BLUE);
    
    text_draw_int_px((HUD_BAR_X + 3) * 8 + 80, (HUD_BAR_Y + 1) * 8, pl->mp, TEXT_COLOR_WHITE);
}

/**
 * Draw floor, level, and gold info.
 */
void hud_draw_info(void) {
    Player* pl = player_get();
    DungeonData* dung = dungeon_get();
    
    /* Floor number */
    text_draw(HUD_FLOOR_X, HUD_FLOOR_Y, "F:", TEXT_COLOR_WHITE);
    text_draw_int(HUD_FLOOR_X + 2, HUD_FLOOR_Y, dung->floor_num, TEXT_COLOR_WHITE);
    
    /* Player level */
    text_draw(HUD_FLOOR_X + 4, HUD_FLOOR_Y, "LV:", TEXT_COLOR_YELLOW);
    text_draw_int(HUD_FLOOR_X + 7, HUD_FLOOR_Y, pl->level, TEXT_COLOR_YELLOW);
    
    /* Gold */
    text_draw(HUD_GOLD_X, HUD_GOLD_Y, "G:", TEXT_COLOR_ORANGE);
    text_draw_int(HUD_GOLD_X + 2, HUD_GOLD_Y, pl->gold, TEXT_COLOR_ORANGE);
    
    /* XP bar (small) */
    text_draw(HUD_FLOOR_X, HUD_FLOOR_Y + 1, "XP:", TEXT_COLOR_GREEN);
    u8 xp_pct = 0;
    if (pl->xp_to_next > 0) {
        xp_pct = (u8)((u32)pl->xp * 10 / (u32)pl->xp_to_next);
    }
    char xp_bar[12];
    int xi = 0;
    for (int i = 0; i < 10; i++) {
        xp_bar[xi++] = (i < xp_pct) ? '#' : '.';
    }
    xp_bar[xi] = '\0';
    text_draw(HUD_FLOOR_X + 3, HUD_FLOOR_Y + 1, xp_bar, TEXT_COLOR_GREEN);
}

/**
 * Draw mini-map.
 * Renders a tiny overview of revealed tiles.
 * Each map tile = 1 pixel (64×64 = 64×64 pixels = 8×8 tiles).
 * We use text characters: '#' for walls, '.' for floor, '@' for player.
 */
void hud_draw_minimap(void) {
    TileMap* map = tilemap_get();
    Player* pl = player_get();
    DungeonData* dung = dungeon_get();
    
    /* Title */
    text_draw(HUD_MINIMAP_X, HUD_MINIMAP_Y - 1, "[MAP]", TEXT_COLOR_GRAY);
    
    /* Draw minimap at character level — each char = 4×4 map tiles (16×16 chars for 64×64) */
    /* To fit in 8×8 tile area, each char represents 8×8 map tiles = 8×8 chars */
    u8 scale = 8;  /* Each char represents 8×8 map tiles */
    
    for (u8 my = 0; my < 8; my++) {
        for (u8 mx = 0; mx < 8; mx++) {
            /* Sample center of each mini-cell */
            u8 tx = mx * scale + scale / 2;
            u8 ty = my * scale + scale / 2;
            
            if (tx >= MAP_WIDTH || ty >= MAP_HEIGHT) {
                text_draw_char(' ', HUD_MINIMAP_X + mx, HUD_MINIMAP_Y + my, TEXT_COLOR_BLACK);
                continue;
            }
            
            if (!map->revealed[ty][tx]) {
                text_draw_char(' ', HUD_MINIMAP_X + mx, HUD_MINIMAP_Y + my, TEXT_COLOR_BLACK);
            } else if (pl->gx / scale == mx && pl->gy / scale == my) {
                /* Player position */
                text_draw_char('@', HUD_MINIMAP_X + mx, HUD_MINIMAP_Y + my, TEXT_COLOR_WHITE);
            } else if (map->tiles[ty][tx] == TILE_WALL || map->tiles[ty][tx] == TILE_WALL2) {
                text_draw_char('#', HUD_MINIMAP_X + mx, HUD_MINIMAP_Y + my, TEXT_COLOR_DARK_GRAY);
            } else {
                text_draw_char('.', HUD_MINIMAP_X + mx, HUD_MINIMAP_Y + my, TEXT_COLOR_GRAY);
            }
        }
    }
}

/**
 * Draw message log.
 */
void hud_draw_messages(void) {
    if (s_msg_count == 0) return;
    
    /* Show the last HUD_MSG_LINES messages */
    u8 start;
    if (s_msg_count <= HUD_MSG_LINES) {
        start = 0;
    } else {
        start = (s_msg_head - HUD_MSG_LINES) % MSG_LOG_SIZE;
    }
    
    for (u8 i = 0; i < HUD_MSG_LINES; i++) {
        u8 idx = (start + i) % MSG_LOG_SIZE;
        if (s_msg_log[idx][0] != '\0') {
            /* Color: most recent message is brighter */
            u8 color = (i == MIN(s_msg_count, HUD_MSG_LINES) - 1) ? 
                       TEXT_COLOR_WHITE : TEXT_COLOR_GRAY;
            text_draw(HUD_MSG_X, HUD_MSG_Y + i, s_msg_log[idx], color);
        }
    }
}

/**
 * Add a message to the log.
 */
void hud_add_message(const char* str) {
    if (!str) return;
    
    /* Copy string to log */
    u8 i = 0;
    while (i < MSG_MAX_LEN - 1 && str[i] != '\0') {
        s_msg_log[s_msg_head][i] = str[i];
        i++;
    }
    s_msg_log[s_msg_head][i] = '\0';
    
    s_msg_head = (s_msg_head + 1) % MSG_LOG_SIZE;
    if (s_msg_count < MSG_LOG_SIZE) s_msg_count++;
}

/**
 * Clear messages.
 */
void hud_clear_messages(void) {
    s_msg_head = 0;
    s_msg_count = 0;
    for (int i = 0; i < MSG_LOG_SIZE; i++) {
        s_msg_log[i][0] = '\0';
    }
}

/**
 * Show overlay text.
 */
void hud_show_overlay(const char* text, u16 timer) {
    u8 i = 0;
    while (i < 19 && text[i] != '\0') {
        s_overlay_text[i] = text[i];
        i++;
    }
    s_overlay_text[i] = '\0';
    s_overlay_timer = timer;
}

/**
 * Clear overlay.
 */
void hud_clear_overlay(void) {
    s_overlay_timer = 0;
    s_overlay_text[0] = '\0';
}
