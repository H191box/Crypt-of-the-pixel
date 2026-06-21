/**
 * game_state.c — Game State Machine Implementation
 * 
 * Orchestrates all game subsystems: dungeon generation,
 * player movement, combat, HUD rendering, and state transitions.
 */

#include "gba_header.h"
#include "gba_types.h"
#include "system.h"
#include "input.h"
#include "random.h"
#include "iso_math.h"
#include "tilemap.h"
#include "sprite_manager.h"
#include "text_renderer.h"
#include "dungeon_gen.h"
#include "player.h"
#include "enemy.h"
#include "combat.h"
#include "items.h"
#include "hud.h"
#include "game_state.h"

/* Forward declarations */
static void update_title(void);
static void update_class_select(void);
static void update_dungeon(void);
static void update_combat(void);
static void update_inventory(void);
static void update_game_over(void);
static void update_victory(void);
static void render_title(void);
static void render_class_select(void);
static void render_dungeon(void);
static void render_combat(void);
static void render_inventory(void);
static void render_game_over(void);
static void render_victory(void);
static void render_game_sprites(void);
extern void load_all_palettes(void);
extern void load_all_tiles(void);
extern void load_all_sprites(void);

/* ---- Current state ---- */
static GameState s_state = STATE_TITLE;

/* ---- Game session data ---- */
static u32 s_dungeon_seed;
static u8 s_floor_num;
static u8 s_player_class;

/* ---- Turn management ---- */
static bool s_player_turn;      /* Is it the player's turn? */
static u16 s_turn_delay;       /* Delay between player and enemy turns */
static u16 s_anim_timer;        /* Timer for animations/transitions */

/* ---- Title screen blink ---- */
static u16 s_title_blink;

/* ---- Class selection ---- */
static u8 s_class_cursor;

/* ---- Combat mode ---- */
static s8 s_combat_enemy;
static u8 s_combat_menu_cursor;

/* ---- Inventory mode ---- */
static u8 s_inv_cursor;

/* ---- Floor transition ---- */
static bool s_floor_transition;
static u16 s_floor_transition_timer;

/* ============================================================
 * INITIALIZATION
 * ============================================================ */

/**
 * Initialize the game.
 */
void game_init(void) {
    /* Init all subsystems */
    sys_init();
    input_init();
    timer_init();
    iso_camera_init();
    tilemap_init();
    sprite_init();
    text_init();
    random_seed(42);
    
    /* Init game subsystems */
    combat_init();
    items_init();
    enemy_init();
    hud_init();
    
    /* Load palette data */
    load_all_palettes();
    load_all_tiles();
    load_all_sprites();
    
    /* Start at title screen */
    s_state = STATE_TITLE;
    s_title_blink = 0;
    s_anim_timer = 0;
    s_floor_transition = FALSE;
}

/* ============================================================
 * UPDATE
 * ============================================================ */

/**
 * Main update loop.
 */
void game_update(void) {
    input_update();
    timer_update();
    
    s_anim_timer++;
    
    switch (s_state) {
        case STATE_TITLE:
            update_title();
            break;
        case STATE_CLASS_SEL:
            update_class_select();
            break;
        case STATE_DUNGEON:
            update_dungeon();
            break;
        case STATE_COMBAT:
            update_combat();
            break;
        case STATE_INVENTORY:
            update_inventory();
            break;
        case STATE_GAME_OVER:
            update_game_over();
            break;
        case STATE_VICTORY:
            update_victory();
            break;
        default:
            break;
    }
}

/* ---- Title screen ---- */
static void update_title(void) {
    s_title_blink++;
    
    if (key_just_pressed(KEY_START)) {
        game_change_state(STATE_CLASS_SEL);
        s_class_cursor = 0;
    }
    
    /* Check for save file */
    if (key_just_pressed(KEY_A)) {
        if (game_load()) {
            game_change_state(STATE_DUNGEON);
        }
    }
}

/* ---- Class selection ---- */
static void update_class_select(void) {
    if (key_just_pressed(KEY_UP)) {
        s_class_cursor = (s_class_cursor == 0) ? 2 : s_class_cursor - 1;
    }
    if (key_just_pressed(KEY_DOWN)) {
        s_class_cursor = (s_class_cursor + 1) % 3;
    }
    
    if (key_just_pressed(KEY_A)) {
        game_new_game(s_class_cursor);
    }
    if (key_just_pressed(KEY_B)) {
        game_change_state(STATE_TITLE);
    }
}

/* ---- Dungeon exploration ---- */
static void update_dungeon(void) {
    /* Handle floor transition */
    if (s_floor_transition) {
        s_floor_transition_timer--;
        if (s_floor_transition_timer == 0) {
            s_floor_transition = FALSE;
            game_next_floor();
        }
        return;
    }
    
    Player* pl = player_get();
    if (!pl->alive) {
        game_change_state(STATE_GAME_OVER);
        return;
    }
    
    /* Check for stairs */
    if (tilemap_get_tile(pl->gx, pl->gy) == TILE_STAIRS) {
        s_floor_transition = TRUE;
        s_floor_transition_timer = 30;
        hud_add_message("Descending to next floor...");
        return;
    }
    
    /* Check for item pickup */
    u8 picked = item_pickup_at_player();
    if (picked != 0) {
        const char* name = item_get_def(picked) ? item_get_def(picked)->name : "???";
        char msg[30];
        /* Simplified: just show item type number */
        msg[0] = 'P'; msg[1] = 'i'; msg[2] = 'c'; msg[3] = 'k';
        msg[4] = 'e'; msg[5] = 'd'; msg[6] = ':'; msg[7] = ' ';
        msg[8] = '\0';
        hud_add_message(msg);
    }
    
    /* Player movement (D-pad) */
    if (!pl->acted) {
        s8 dx = 0, dy = 0;
        
        if (key_just_pressed(KEY_UP))    dy = -1;
        if (key_just_pressed(KEY_DOWN))  dy = 1;
        if (key_just_pressed(KEY_LEFT))  dx = -1;
        if (key_just_pressed(KEY_RIGHT)) dx = 1;
        
        if (dx != 0 || dy != 0) {
            if (player_move(dx, dy)) {
                /* Successful move — trigger enemy turns after delay */
                s_turn_delay = 5;  /* 5 frame delay before enemies act */
                
                /* Reveal tiles around new position */
                tilemap_clear_visibility();
                tilemap_reveal_around(pl->gx, pl->gy, 7);
                
                /* Center camera on player */
                iso_camera_center(pl->gx, pl->gy);
            } else {
                /* Move failed — check if enemy is in that direction */
                u8 tx = (u8)((s16)pl->gx + dx);
                u8 ty = (u8)((s16)pl->gy + dy);
                s8 eid = enemy_at(tx, ty);
                if (eid >= 0) {
                    /* Start combat */
                    s_combat_enemy = eid;
                    combat_start(eid);
                    game_change_state(STATE_COMBAT);
                    return;
                }
            }
        }
        
        /* Attack button (A) — attack adjacent enemy */
        if (key_just_pressed(KEY_A)) {
            s16 dmg = player_attack();
            if (dmg >= 0) {
                char msg[20];
                msg[0] = 'H'; msg[1] = 'i'; msg[2] = 't'; msg[3] = '!';
                msg[4] = ' '; msg[5] = '\0';
                hud_add_message(msg);
                s_turn_delay = 5;
            }
        }
        
        /* Magic button (B) */
        if (key_just_pressed(KEY_B)) {
            s16 dmg = player_cast_magic(0);
            if (dmg >= 0) {
                hud_add_message("Fireball!");
                s_turn_delay = 5;
            }
        }
        
        /* Open inventory (SELECT) */
        if (key_just_pressed(KEY_SELECT)) {
            s_inv_cursor = 0;
            game_change_state(STATE_INVENTORY);
            return;
        }
        
        /* Quick save (START) */
        if (key_just_pressed(KEY_START)) {
            game_save();
            hud_add_message("Game saved!");
        }
    }
    
    /* Process turn delay (enemies act after player) */
    if (s_turn_delay > 0) {
        s_turn_delay--;
        if (s_turn_delay == 0) {
            enemy_update_all();
            player_new_turn();
            
            /* Check if player died from enemy attacks */
            if (!pl->alive) {
                game_change_state(STATE_GAME_OVER);
            }
        }
    }
    
    /* Update camera smooth scrolling */
    iso_camera_update();
    
    /* Update sprites */
    sprite_update();
}

/* ---- Combat mode ---- */
static void update_combat(void) {
    CombatState cs = combat_get_state();
    
    if (cs == COMBAT_IDLE) {
        game_change_state(STATE_DUNGEON);
        return;
    }
    
    if (cs == COMBAT_RESULT) {
        CombatResult res = combat_get_result();
        
        if (res.enemy_killed) {
            hud_add_message("Enemy defeated!");
            combat_end();
            game_change_state(STATE_DUNGEON);
            return;
        }
        
        if (!player_get()->alive) {
            hud_add_message("You have fallen...");
            combat_end();
            game_change_state(STATE_GAME_OVER);
            return;
        }
        
        if (res.flee_success) {
            hud_add_message("Fled successfully!");
            combat_end();
            game_change_state(STATE_DUNGEON);
            return;
        }
        
        /* Combat continues — new round */
        combat_player_action(ACTION_ATTACK);
        return;
    }
    
    if (cs == COMBAT_PLAYER_TURN) {
        if (key_just_pressed(KEY_UP)) {
            combat_player_action(ACTION_ATTACK);
        }
        if (key_just_pressed(KEY_DOWN)) {
            combat_player_action(ACTION_MAGIC);
        }
        if (key_just_pressed(KEY_LEFT) || key_just_pressed(KEY_RIGHT)) {
            combat_player_action(ACTION_FLEE);
        }
        if (key_just_pressed(KEY_A)) {
            combat_player_action(ACTION_ATTACK);
        }
        if (key_just_pressed(KEY_B)) {
            combat_player_action(ACTION_FLEE);
        }
    }
}

/* ---- Inventory screen ---- */
static void update_inventory(void) {
    Player* pl = player_get();
    
    if (key_just_pressed(KEY_UP) && s_inv_cursor > 0) s_inv_cursor--;
    if (key_just_pressed(KEY_DOWN) && s_inv_cursor < MAX_INVENTORY - 1) s_inv_cursor++;
    
    if (key_just_pressed(KEY_A)) {
        player_use_item(s_inv_cursor);
    }
    
    if (key_just_pressed(KEY_B) || key_just_pressed(KEY_SELECT)) {
        game_change_state(STATE_DUNGEON);
    }
}

/* ---- Game over ---- */
static void update_game_over(void) {
    if (key_just_pressed(KEY_START)) {
        game_change_state(STATE_TITLE);
    }
}

/* ---- Victory ---- */
static void update_victory(void) {
    if (key_just_pressed(KEY_START)) {
        game_change_state(STATE_TITLE);
    }
}

/* ============================================================
 * RENDER
 * ============================================================ */

/**
 * Main render function.
 */
void game_render(void) {
    switch (s_state) {
        case STATE_TITLE:
            render_title();
            break;
        case STATE_CLASS_SEL:
            render_class_select();
            break;
        case STATE_DUNGEON:
            render_dungeon();
            break;
        case STATE_COMBAT:
            render_combat();
            break;
        case STATE_INVENTORY:
            render_inventory();
            break;
        case STATE_GAME_OVER:
            render_game_over();
            break;
        case STATE_VICTORY:
            render_victory();
            break;
    }
}

/* ---- Title screen render ---- */
static void render_title(void) {
    text_clear_all();
    text_draw_px(40, 20, "CRYPT OF THE", TEXT_COLOR_WHITE);
    text_draw_px(44, 36, "PIXEL", TEXT_COLOR_YELLOW);
    text_draw_px(52, 56, "GBA", TEXT_COLOR_GRAY);
    
    if ((s_title_blink / 30) % 2 == 0) {
        text_draw_px(56, 100, "PRESS START", TEXT_COLOR_WHITE);
    }
    
    text_draw_px(48, 120, "A:Load B:New", TEXT_COLOR_GRAY);
    text_flush();
}

/* ---- Class selection render ---- */
static void render_class_select(void) {
    text_clear_all();
    text_draw_px(28, 10, "CHOOSE CLASS", TEXT_COLOR_WHITE);
    
    const char* classes[] = {"WARRIOR", "MAGE", "THIEF"};
    const char* descs[] = {
        "Hi HP/ATK",
        "Hi Magic",
        "Hi Evade"
    };
    
    for (int i = 0; i < 3; i++) {
        u8 color = (i == s_class_cursor) ? TEXT_COLOR_YELLOW : TEXT_COLOR_GRAY;
        text_draw_px(60, 40 + i * 24, classes[i], color);
        text_draw_px(60, 48 + i * 24, descs[i], color);
    }
    
    text_draw_px(48, 130, "A:Select B:Back", TEXT_COLOR_GRAY);
    text_flush();
}

/* ---- Dungeon render ---- */
static void render_dungeon(void) {
    /* Render tilemap */
    tilemap_render();
    
    /* Render sprites (player, enemies, items) */
    render_game_sprites();
    sprite_render();
    
    /* Render HUD */
    hud_update();
}

/* ---- Combat render ---- */
static void render_combat(void) {
    tilemap_render();
    render_game_sprites();
    sprite_render();
    hud_update();
    
    /* Combat overlay */
    text_draw_px(4, 70, "COMBAT!", TEXT_COLOR_RED);
    text_draw_px(4, 82, "U:Atk D:Mag", TEXT_COLOR_WHITE);
    text_draw_px(4, 92, "L/R:Flee", TEXT_COLOR_WHITE);
    text_flush();
}

/* ---- Inventory render ---- */
static void render_inventory(void) {
    text_clear_all();
    text_draw_px(56, 8, "INVENTORY", TEXT_COLOR_WHITE);
    
    Player* pl = player_get();
    for (int i = 0; i < MAX_INVENTORY; i++) {
        u8 y = (u8)(16 + i * 8);
        u8 color = (i == s_inv_cursor) ? TEXT_COLOR_YELLOW : TEXT_COLOR_GRAY;
        
        if (pl->inventory[i].type != 0) {
            /* Show item number and count */
            char buf[10];
            buf[0] = 'I';
            buf[1] = 'T';
            buf[2] = 'E';
            buf[3] = 'M';
            buf[4] = ' ';
            buf[5] = '0' + pl->inventory[i].type;
            buf[6] = 'x';
            buf[7] = '0' + pl->inventory[i].count;
            buf[8] = '\0';
            text_draw_px(20, y, buf, color);
        }
    }
    
    text_draw_px(40, 140, "A:Use B:Exit", TEXT_COLOR_GRAY);
    text_flush();
}

/* ---- Game over render ---- */
static void render_game_over(void) {
    text_clear_all();
    text_draw_px(48, 50, "GAME OVER", TEXT_COLOR_RED);
    text_draw_px(56, 80, "YOU DIED", TEXT_COLOR_GRAY);
    
    if ((s_anim_timer / 30) % 2 == 0) {
        text_draw_px(48, 120, "PRESS START", TEXT_COLOR_WHITE);
    }
    text_flush();
}

/* ---- Victory render ---- */
static void render_victory(void) {
    text_clear_all();
    text_draw_px(36, 40, "VICTORY!", TEXT_COLOR_YELLOW);
    text_draw_px(20, 60, "You conquered", TEXT_COLOR_WHITE);
    text_draw_px(20, 70, "the Crypt!", TEXT_COLOR_WHITE);
    
    Player* pl = player_get();
    char buf[20];
    buf[0] = 'L'; buf[1] = 'v'; buf[2] = ':'; buf[3] = ' ';
    buf[4] = '0' + pl->level; buf[5] = ' '; buf[6] = '\0';
    text_draw_px(60, 90, buf, TEXT_COLOR_WHITE);
    
    if ((s_anim_timer / 30) % 2 == 0) {
        text_draw_px(48, 120, "PRESS START", TEXT_COLOR_WHITE);
    }
    text_flush();
}

/* ---- Render game sprites (player, enemies, items) ---- */
static void render_game_sprites(void) {
    Player* pl = player_get();
    IsoCamera* cam = iso_get_camera();
    
    /* Calculate player screen position */
    s16 sx, sy;
    iso_to_screen(pl->gx, pl->gy, &sx, &sy);
    
    /* Set depth for sorting */
    s16 depth = pl->gx + pl->gy;
    
    /* Position player sprite (lifted up by tile height) */
    /* Player sprite should be at tile center, raised above floor */
    
    /* Update enemy sprites */
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* e = enemy_get(i);
        if (!e) continue;
        
        if (tilemap_is_visible(e->gx, e->gy)) {
            s16 esx, esy;
            iso_to_screen(e->gx, e->gy, &esx, &esy);
            /* Update sprite position would go here */
            /* sprite_set_pos(e->sprite_slot, esx - 4, esy - 8); */
        }
    }
}

/* ============================================================
 * STATE MANAGEMENT
 * ============================================================ */

void game_change_state(GameState new_state) {
    s_state = new_state;
}

GameState game_get_state(void) {
    return s_state;
}

/* ============================================================
 * SAVE / LOAD
 * ============================================================ */

bool game_save(void) {
    Player* pl = player_get();
    DungeonData* dung = dungeon_get();
    
    SaveData save;
    save.magic = SAVE_MAGIC;
    save.version = SAVE_VERSION;
    save.dungeon_seed = dung->seed;
    save.floor_num = dung->floor_num;
    save.player_class = pl->p_class;
    save.player_hp = (u16)pl->hp;
    save.player_max_hp = (u16)pl->max_hp;
    save.player_mp = (u16)pl->mp;
    save.player_max_mp = (u16)pl->max_mp;
    save.player_level = pl->level;
    save.player_xp = pl->xp;
    save.player_gold = pl->gold;
    save.player_gx = pl->gx;
    save.player_gy = pl->gy;
    
    /* Calculate checksum */
    save.checksum = 0;
    u8* ptr = (u8*)&save;
    for (int i = 0; i < (int)sizeof(SaveData) - 2; i++) {
        save.checksum ^= ptr[i];
    }
    
    /* Write to SRAM */
    u8* sram = (u8*)SRAM;
    for (int i = 0; i < (int)sizeof(SaveData); i++) {
        sram[i] = ptr[i];
    }
    
    return TRUE;
}

bool game_load(void) {
    SaveData save;
    u8* sram = (u8*)SRAM;
    
    /* Read from SRAM */
    for (int i = 0; i < (int)sizeof(SaveData); i++) {
        ((u8*)&save)[i] = sram[i];
    }
    
    /* Validate */
    if (save.magic != SAVE_MAGIC || save.version != SAVE_VERSION) {
        return FALSE;
    }
    
    /* Verify checksum */
    u16 checksum = 0;
    u8* ptr = (u8*)&save;
    for (int i = 0; i < (int)sizeof(SaveData) - 2; i++) {
        checksum ^= ptr[i];
    }
    if (checksum != save.checksum) return FALSE;
    
    /* Restore game state */
    s_dungeon_seed = save.dungeon_seed;
    s_floor_num = save.floor_num;
    
    /* Regenerate dungeon */
    tilemap_init();
    enemy_init();
    dungeon_generate(s_dungeon_seed, s_floor_num);
    dungeon_place_tiles();
    
    /* Restore player */
    player_init((PlayerClass)save.player_class);
    Player* pl = player_get();
    pl->hp = save.player_hp;
    pl->max_hp = save.player_max_hp;
    pl->mp = save.player_mp;
    pl->max_mp = save.player_max_mp;
    pl->level = save.player_level;
    pl->xp = save.player_xp;
    pl->gold = save.player_gold;
    pl->gx = save.player_gx;
    pl->gy = save.player_gy;
    
    /* Setup view */
    iso_camera_center(pl->gx, pl->gy);
    tilemap_reveal_around(pl->gx, pl->gy, 7);
    enemy_populate_dungeon();
    
    return TRUE;
}

bool game_has_save(void) {
    SaveData save;
    u8* sram = (u8*)SRAM;
    for (int i = 0; i < (int)sizeof(SaveData); i++) {
        ((u8*)&save)[i] = sram[i];
    }
    return (save.magic == SAVE_MAGIC && save.version == SAVE_VERSION) ? TRUE : FALSE;
}

/* ============================================================
 * GAME FLOW
 * ============================================================ */

void game_new_game(u8 player_class) {
    s_player_class = player_class;
    s_floor_num = 1;
    s_dungeon_seed = (u32)rand_next();
    
    /* Generate first floor */
    tilemap_init();
    enemy_init();
    items_init();
    
    DungeonData* dung = dungeon_generate(s_dungeon_seed, s_floor_num);
    dungeon_place_tiles();
    
    /* Init player */
    player_init((PlayerClass)player_class);
    Player* pl = player_get();
    
    /* Place player at spawn */
    u8 sx, sy;
    dungeon_get_spawn(&sx, &sy);
    pl->gx = sx;
    pl->gy = sy;
    
    /* Reveal around player */
    tilemap_reveal_around(pl->gx, pl->gy, 7);
    
    /* Center camera */
    iso_camera_center(pl->gx, pl->gy);
    
    /* Populate enemies */
    enemy_populate_dungeon();
    
    /* Change to dungeon state */
    game_change_state(STATE_DUNGEON);
    hud_add_message("Floor 1 - Begin!");
}

void game_next_floor(void) {
    s_floor_num++;
    
    if (s_floor_num > MAX_FLOOR) {
        game_change_state(STATE_VICTORY);
        return;
    }
    
    /* Generate new dungeon with related seed */
    s_dungeon_seed += 0x12345678;
    tilemap_init();
    enemy_init();
    items_init();
    
    DungeonData* dung = dungeon_generate(s_dungeon_seed, s_floor_num);
    dungeon_place_tiles();
    
    /* Place player at new spawn */
    Player* pl = player_get();
    u8 sx, sy;
    dungeon_get_spawn(&sx, &sy);
    pl->gx = sx;
    pl->gy = sy;
    
    /* Restore HP/MP partially */
    player_heal(pl->max_hp / 4);
    player_restore_mp(pl->max_mp / 4);
    
    tilemap_reveal_around(pl->gx, pl->gy, 7);
    iso_camera_center(pl->gx, pl->gy);
    enemy_populate_dungeon();
    
    char msg[20];
    msg[0] = 'F'; msg[1] = 'l'; msg[2] = 'o'; msg[3] = 'o';
    msg[4] = 'r'; msg[5] = ' '; msg[6] = '0' + s_floor_num;
    msg[7] = '\0';
    hud_add_message(msg);
}
