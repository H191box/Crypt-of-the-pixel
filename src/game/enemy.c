/**
 * enemy.c — Enemy System Implementation
 * 
 * Simple turn-based AI:
 * 1. If player is within detection range, move toward player
 * 2. If adjacent to player, attack
 * 3. Otherwise, random patrol movement
 * 
 * Enemies are placed during dungeon generation based on room types
 * and floor number.
 */

#include "gba_types.h"
#include "random.h"
#include "tilemap.h"
#include "player.h"
#include "dungeon_gen.h"
#include "enemy.h"

/* ---- Enemy pool ---- */
static Enemy s_enemies[MAX_ENEMIES];

/* ---- Enemy base stats table ---- */
/* { hp, atk, def, spd, xp, gold, sprite_base } */
static const s16 s_enemy_stats[ENEMY_MAX][7] = {
    /* 0: SLIME     */ {  8,  3,  1, 1,  5,  3, 0 },
    /* 1: BAT       */ {  5,  4,  0, 2,  4,  2, 3 },
    /* 2: SKELETON  */ { 15,  5,  3, 1, 10,  8, 6 },
    /* 3: GHOST     */ { 12,  6,  1, 2, 12, 10, 9 },
    /* 4: ORC       */ { 20,  7,  5, 1, 15, 12, 12 },
    /* 5-9: UNUSED  */ { 0, 0, 0, 0, 0, 0, 0 },
    /* 5-9: UNUSED  */ { 0, 0, 0, 0, 0, 0, 0 },
    /* 5-9: UNUSED  */ { 0, 0, 0, 0, 0, 0, 0 },
    /* 5-9: UNUSED  */ { 0, 0, 0, 0, 0, 0, 0 },
    /* 5-9: UNUSED  */ { 0, 0, 0, 0, 0, 0, 0 },
    /* 10: BOSS_SLIME   */ { 50, 10,  5, 1, 50, 100, 15 },
    /* 11: BOSS_SKELE   */ { 60, 12,  8, 1, 60, 120, 17 },
    /* 12: BOSS_DEMON   */ { 80, 15, 10, 2, 80, 150, 19 },
};

/**
 * Initialize enemy system.
 */
void enemy_init(void) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        s_enemies[i].active = FALSE;
    }
}

/**
 * Create a new enemy.
 */
s8 enemy_create(EnemyType type, u8 gx, u8 gy) {
    if (type >= ENEMY_MAX) return -1;
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!s_enemies[i].active) {
            Enemy* e = &s_enemies[i];
            e->active = TRUE;
            e->type = type;
            e->gx = gx;
            e->gy = gy;
            
            /* Floor-based scaling */
            DungeonData* dung = dungeon_get();
            u8 floor_scale = dung->floor_num - 1;  /* 0 for floor 1 */
            
            e->max_hp = s_enemy_stats[type][0] + floor_scale * 3;
            e->hp = e->max_hp;
            e->attack = (u8)(s_enemy_stats[type][1] + floor_scale);
            e->defense = (u8)(s_enemy_stats[type][2] + floor_scale / 2);
            e->speed = (u8)s_enemy_stats[type][3];
            e->xp_reward = (u8)(s_enemy_stats[type][4] + floor_scale * 2);
            e->gold_reward = (u8)(s_enemy_stats[type][5] + floor_scale * 3);
            e->sprite_id = (u8)s_enemy_stats[type][6];
            e->anim_frame = 0;
            e->move_timer = 0;
            e->turn_count = 0;
            
            return i;
        }
    }
    return -1;
}

/**
 * Remove an enemy.
 */
void enemy_remove(s8 id) {
    if (id < 0 || id >= MAX_ENEMIES) return;
    s_enemies[id].active = FALSE;
}

/**
 * Get enemy pointer.
 */
Enemy* enemy_get(s8 id) {
    if (id < 0 || id >= MAX_ENEMIES) return NULL;
    if (!s_enemies[id].active) return NULL;
    return &s_enemies[id];
}

/**
 * Update all enemies (their turn).
 * Simple AI:
 * 1. If adjacent to player → attack
 * 2. If within DETECT_RANGE → pathfind toward player (simple: move to reduce distance)
 * 3. Else → random move or stay
 */
void enemy_update_all(void) {
    Player* pl = player_get();
    if (!pl->alive) return;
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* e = &s_enemies[i];
        if (!e->active) continue;
        
        e->turn_count++;
        
        /* Speed check: some enemies skip turns */
        e->move_timer++;
        if (e->move_timer < e->speed) continue;
        e->move_timer = 0;
        
        /* Calculate distance to player */
        s16 dx = (s16)pl->gx - (s16)e->gx;
        s16 dy = (s16)pl->gy - (s16)e->gy;
        u8 dist = (u8)(ABS(dx) + ABS(dy));
        
        /* Adjacent? Attack! */
        if (dist == 1) {
            s16 dmg = e->attack + (s16)rand_range(-1, 2);
            if (dmg < 1) dmg = 1;
            player_take_damage(dmg);
            continue;
        }
        
        /* Within detection range? Move toward player */
        if (dist <= DETECT_RANGE && tilemap_is_visible(e->gx, e->gy)) {
            s8 move_x = 0, move_y = 0;
            
            /* Move along the axis with greater distance */
            if (ABS(dx) >= ABS(dy)) {
                move_x = (dx > 0) ? 1 : -1;
            } else {
                move_y = (dy > 0) ? 1 : -1;
            }
            
            u8 nx = (u8)((s16)e->gx + move_x);
            u8 ny = (u8)((s16)e->gy + move_y);
            
            /* Check if move is valid */
            if (nx < MAP_WIDTH && ny < MAP_HEIGHT && 
                tilemap_is_walkable(nx, ny) && enemy_at(nx, ny) < 0 &&
                !(nx == pl->gx && ny == pl->gy)) {
                e->gx = nx;
                e->gy = ny;
            }
        } else {
            /* Random patrol: 30% chance to move in a random direction */
            if (rand_chance(30)) {
                s8 dirs[4][2] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };
                u8 dir = (u8)rand_int(4);
                u8 nx = (u8)((s16)e->gx + dirs[dir][0]);
                u8 ny = (u8)((s16)e->gy + dirs[dir][1]);
                
                if (nx < MAP_WIDTH && ny < MAP_HEIGHT &&
                    tilemap_is_walkable(nx, ny) && enemy_at(nx, ny) < 0) {
                    e->gx = nx;
                    e->gy = ny;
                }
            }
        }
        
        /* Advance animation frame */
        e->anim_frame = (e->anim_frame + 1) % 2;
    }
}

/**
 * Damage an enemy. Returns remaining HP or -1 if dead.
 */
s16 enemy_damage(s8 id, s16 amount) {
    if (id < 0 || id >= MAX_ENEMIES) return -1;
    Enemy* e = &s_enemies[id];
    if (!e->active) return -1;
    
    s16 dmg = amount - e->defense / 2;
    if (dmg < 1) dmg = 1;
    
    e->hp -= dmg;
    
    if (e->hp <= 0) {
        /* Enemy dies */
        Player* pl = player_get();
        player_gain_xp(e->xp_reward);
        pl->gold += e->gold_reward;
        
        e->active = FALSE;
        return -1;
    }
    
    return e->hp;
}

/**
 * Check if enemy is at position.
 */
s8 enemy_at(u8 x, u8 y) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (s_enemies[i].active && s_enemies[i].gx == x && s_enemies[i].gy == y) {
            return i;
        }
    }
    return -1;
}

/**
 * Count active enemies.
 */
u8 enemy_count(void) {
    u8 count = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (s_enemies[i].active) count++;
    }
    return count;
}

/**
 * Populate dungeon rooms with enemies.
 */
void enemy_populate_dungeon(void) {
    DungeonData* dung = dungeon_get();
    
    /* Enemies per room: 1-3 for normal rooms, 0 for treasure */
    for (u8 r = 0; r < dung->room_count; r++) {
        Room* room = &dung->rooms[r];
        
        if (room->type == ROOM_SPAWN) continue;
        
        u8 count = 0;
        EnemyType type;
        
        switch (room->type) {
            case ROOM_BOSS:
                /* Boss enemy */
                if (dung->floor_num <= 3) {
                    type = ENEMY_BOSS_SLIME;
                } else if (dung->floor_num <= 6) {
                    type = ENEMY_BOSS_SKELE;
                } else {
                    type = ENEMY_BOSS_DEMON;
                }
                enemy_create(type, room->cx, room->cy);
                break;
                
            case ROOM_TREASURE:
                /* No enemies in treasure rooms */
                break;
                
            default:
                /* Normal room: 1-3 enemies based on floor */
                count = (u8)rand_range(1, MIN(3, 1 + dung->floor_num / 3));
                
                /* Enemy type based on floor */
                for (u8 e = 0; e < count; e++) {
                    u8 roll = (u8)rand_int(100);
                    if (dung->floor_num <= 2) {
                        type = (roll < 60) ? ENEMY_SLIME : ENEMY_BAT;
                    } else if (dung->floor_num <= 4) {
                        if (roll < 30) type = ENEMY_SLIME;
                        else if (roll < 55) type = ENEMY_BAT;
                        else if (roll < 80) type = ENEMY_SKELETON;
                        else type = ENEMY_GHOST;
                    } else {
                        if (roll < 15) type = ENEMY_SLIME;
                        else if (roll < 30) type = ENEMY_BAT;
                        else if (roll < 55) type = ENEMY_SKELETON;
                        else if (roll < 80) type = ENEMY_GHOST;
                        else type = ENEMY_ORC;
                    }
                    
                    /* Random position inside room */
                    u8 ex = (u8)rand_range(room->x + 1, room->x + room->w - 2);
                    u8 ey = (u8)rand_range(room->y + 1, room->y + room->h - 2);
                    
                    /* Don't place on special tiles */
                    if (tilemap_get_tile(ex, ey) == TILE_CHEST ||
                        tilemap_get_tile(ex, ey) == TILE_STAIRS ||
                        tilemap_get_tile(ex, ey) == TILE_ALTAR) {
                        continue;
                    }
                    
                    enemy_create(type, ex, ey);
                }
                break;
        }
    }
}

/**
 * Find nearest alive enemy to position.
 */
s8 enemy_nearest(u8 x, u8 y) {
    s16 best_dist = 9999;
    s8 best_id = -1;
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!s_enemies[i].active) continue;
        s16 dx = ABS((s16)x - (s16)s_enemies[i].gx);
        s16 dy = ABS((s16)y - (s16)s_enemies[i].gy);
        s16 dist = dx + dy;
        if (dist < best_dist) {
            best_dist = dist;
            best_id = i;
        }
    }
    return best_id;
}
