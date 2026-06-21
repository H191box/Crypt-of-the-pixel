/**
 * dungeon_gen.c — BSP Dungeon Generation Implementation
 * 
 * Algorithm:
 * 1. BSP divides the map area recursively until regions are small enough
 * 2. A room is placed in each leaf node
 * 3. Corridors connect adjacent rooms (siblings first, then parent chain)
 * 4. Special rooms (spawn, boss, treasure) are assigned based on distance
 * 
 * The map grid is 64×64 tiles. BSP operates on a sub-region to leave
 * border walls around the edges.
 */

#include "gba_types.h"
#include "random.h"
#include "tilemap.h"
#include "dungeon_gen.h"

/* Forward declarations */
static void bsp_split(u8 node_idx, u8 depth);
static s8 find_room_in_subtree(u8 node_idx);
static void connect_rooms(u8 room_a, u8 room_b);
static void carve_corridor_h(u8 x1, u8 x2, u8 y);
static void carve_corridor_v(u8 y1, u8 y2, u8 x);
static void ensure_all_connected(void);

/* ---- Dungeon data ---- */
static DungeonData s_dungeon;

/* ---- BSP node structure ---- */
typedef struct {
    u8 x, y, w, h;       /* Partition bounds */
    u8 split_dir;         /* 0 = horizontal, 1 = vertical */
    u8 split_pos;         /* Where the partition line is */
    s8 left;              /* Index of left child (-1 = leaf) */
    s8 right;             /* Index of right child (-1 = leaf) */
    s8 room;              /* Index of room placed here (-1 = none) */
} BSPNode;

#define MAX_BSP_NODES  32
static BSPNode s_bsp[MAX_BSP_NODES];
static u8 s_bsp_count;

/**
 * Generate a new dungeon.
 */
DungeonData* dungeon_generate(u32 seed, u8 floor_num) {
    /* Seed RNG */
    rand_seed(seed);
    
    /* Initialize dungeon data */
    s_dungeon.seed = seed;
    s_dungeon.floor_num = floor_num;
    s_dungeon.room_count = 0;
    s_dungeon.spawn_room = 0;
    s_dungeon.boss_room = 0xFF;
    
    /* Clear rooms */
    for (int i = 0; i < MAX_ROOMS; i++) {
        s_dungeon.rooms[i].connected = FALSE;
    }
    
    /* ---- BSP partitioning ---- */
    s_bsp_count = 0;
    
    /* Root node covers the usable map area (leaving 2-tile border) */
    s_bsp[0].x = 2;
    s_bsp[0].y = 2;
    s_bsp[0].w = MAP_WIDTH - 4;
    s_bsp[0].h = MAP_HEIGHT - 4;
    s_bsp[0].split_dir = 0;
    s_bsp[0].split_pos = 0;
    s_bsp[0].left = -1;
    s_bsp[0].right = -1;
    s_bsp[0].room = -1;
    s_bsp_count = 1;
    
    /* Recursively partition */
    bsp_split(0, 0);
    
    /* ---- Place rooms in leaf nodes ---- */
    for (u8 i = 0; i < s_bsp_count; i++) {
        if (s_bsp[i].left == -1 && s_bsp[i].room == -1) {
            /* Leaf node with no room yet */
            if (s_dungeon.room_count < MAX_ROOMS) {
                u8 ridx = s_dungeon.room_count;
                Room* room = &s_dungeon.rooms[ridx];
                
                /* Room size: random within the node bounds */
                u8 min_w = MIN_ROOM_SIZE;
                u8 max_w = MIN(s_bsp[i].w - 2, MAX_ROOM_WIDTH);
                u8 min_h = MIN_ROOM_SIZE;
                u8 max_h = MIN(s_bsp[i].h - 2, MAX_ROOM_HEIGHT);
                
                if (max_w < min_w) max_w = min_w;
                if (max_h < min_h) max_h = min_h;
                
                room->w = (u8)rand_range(min_w, max_w);
                room->h = (u8)rand_range(min_h, max_h);
                
                /* Position within node bounds */
                u8 max_x = s_bsp[i].x + s_bsp[i].w - room->w;
                u8 max_y = s_bsp[i].y + s_bsp[i].h - room->h;
                room->x = (u8)rand_range(s_bsp[i].x + 1, max_x);
                room->y = (u8)rand_range(s_bsp[i].y + 1, max_y);
                
                room->cx = room->x + room->w / 2;
                room->cy = room->y + room->h / 2;
                room->type = ROOM_NORMAL;
                room->connected = FALSE;
                
                s_bsp[i].room = (s8)ridx;
                s_dungeon.room_count++;
            }
        }
    }
    
    /* ---- Connect rooms with corridors ---- */
    /* Connect sibling nodes first, then work up the tree */
    for (u8 i = 0; i < s_bsp_count; i++) {
        if (s_bsp[i].left >= 0 && s_bsp[i].right >= 0) {
            s8 left_room = find_room_in_subtree(s_bsp[i].left);
            s8 right_room = find_room_in_subtree(s_bsp[i].right);
            if (left_room >= 0 && right_room >= 0) {
                connect_rooms(left_room, right_room);
            }
        }
    }
    
    /* Ensure all rooms are connected */
    ensure_all_connected();
    
    /* ---- Assign special room types ---- */
    /* Spawn room: first room generated */
    s_dungeon.spawn_room = 0;
    s_dungeon.rooms[0].type = ROOM_SPAWN;
    
    /* Find the farthest room from spawn for the boss room */
    u8 max_dist = 0;
    for (u8 i = 1; i < s_dungeon.room_count; i++) {
        s16 dx = s_dungeon.rooms[i].cx - s_dungeon.rooms[0].cx;
        s16 dy = s_dungeon.rooms[i].cy - s_dungeon.rooms[0].cy;
        u8 dist = (u8)(ABS(dx) + ABS(dy));
        if (dist > max_dist) {
            max_dist = dist;
            s_dungeon.boss_room = i;
        }
    }
    s_dungeon.rooms[s_dungeon.boss_room].type = ROOM_BOSS;
    
    /* Assign 1-2 treasure rooms (random, not spawn or boss) */
    u8 treasure_count = (u8)rand_range(1, 2);
    for (u8 t = 0; t < treasure_count; t++) {
        for (int attempts = 0; attempts < 20; attempts++) {
            u8 r = (u8)rand_int(s_dungeon.room_count);
            if (r != s_dungeon.spawn_room && r != s_dungeon.boss_room &&
                s_dungeon.rooms[r].type == ROOM_NORMAL) {
                s_dungeon.rooms[r].type = ROOM_TREASURE;
                break;
            }
        }
    }
    
    /* Place stairs in the boss room */
    s_dungeon.stair_x = s_dungeon.rooms[s_dungeon.boss_room].cx;
    s_dungeon.stair_y = s_dungeon.rooms[s_dungeon.boss_room].cy;
    
    return &s_dungeon;
}

/* ---- BSP recursive split ---- */
static void bsp_split(u8 node_idx, u8 depth) {
    BSPNode* node = &s_bsp[node_idx];
    
    /* Check if we should stop splitting */
    u8 area = node->w * node->h;
    u8 min_dim = MIN(node->w, node->h);
    
    if (depth >= BSP_MAX_DEPTH || area < BSP_MIN_SPLIT || 
        min_dim < MIN_ROOM_SIZE * 2 + 2 || s_bsp_count >= MAX_BSP_NODES - 2) {
        return;  /* Leaf node */
    }
    
    /* Choose split direction: prefer splitting the longer dimension */
    bool can_split_h = (node->h >= MIN_ROOM_SIZE * 2 + 2);
    bool can_split_v = (node->w >= MIN_ROOM_SIZE * 2 + 2);
    
    u8 dir;
    if (can_split_h && can_split_v) {
        dir = (node->w > node->h) ? 1 : 0;  /* Split longer axis */
    } else if (can_split_h) {
        dir = 0;  /* Horizontal split */
    } else if (can_split_v) {
        dir = 1;  /* Vertical split */
    } else {
        return;  /* Can't split */
    }
    
    node->split_dir = dir;
    
    /* Create child nodes */
    s8 left_idx = (s8)s_bsp_count++;
    s8 right_idx = (s8)s_bsp_count++;
    node->left = left_idx;
    node->right = right_idx;
    
    BSPNode* left = &s_bsp[left_idx];
    BSPNode* right = &s_bsp[right_idx];
    
    if (dir == 0) {
        /* Horizontal split (divide height) */
        u8 split = (u8)rand_range(node->y + MIN_ROOM_SIZE + 1, 
                                    node->y + node->h - MIN_ROOM_SIZE - 1);
        node->split_pos = split;
        
        left->x = node->x;
        left->y = node->y;
        left->w = node->w;
        left->h = split - node->y;
        
        right->x = node->x;
        right->y = split;
        right->w = node->w;
        right->h = node->y + node->h - split;
    } else {
        /* Vertical split (divide width) */
        u8 split = (u8)rand_range(node->x + MIN_ROOM_SIZE + 1,
                                    node->x + node->w - MIN_ROOM_SIZE - 1);
        node->split_pos = split;
        
        left->x = node->x;
        left->y = node->y;
        left->w = split - node->x;
        left->h = node->h;
        
        right->x = split;
        right->y = node->y;
        right->w = node->x + node->w - split;
        right->h = node->h;
    }
    
    left->left = -1; left->right = -1; left->room = -1;
    right->left = -1; right->right = -1; right->room = -1;
    
    /* Recurse on children */
    bsp_split((u8)left_idx, depth + 1);
    bsp_split((u8)right_idx, depth + 1);
}

/* ---- Find the room index in a subtree ---- */
static s8 find_room_in_subtree(u8 node_idx) {
    BSPNode* node = &s_bsp[node_idx];
    if (node->room >= 0) return node->room;
    
    if (node->left >= 0) {
        s8 r = find_room_in_subtree((u8)node->left);
        if (r >= 0) return r;
    }
    if (node->right >= 0) {
        s8 r = find_room_in_subtree((u8)node->right);
        if (r >= 0) return r;
    }
    
    return -1;
}

/* ---- Connect two rooms with an L-shaped corridor ---- */
static void connect_rooms(u8 room_a, u8 room_b) {
    Room* a = &s_dungeon.rooms[room_a];
    Room* b = &s_dungeon.rooms[room_b];
    
    u8 x1 = a->cx;
    u8 y1 = a->cy;
    u8 x2 = b->cx;
    u8 y2 = b->cy;
    
    /* L-shaped corridor: go horizontal first, then vertical (or vice versa) */
    if (rand_chance(50)) {
        carve_corridor_h(x1, x2, y1);
        carve_corridor_v(y1, y2, x2);
    } else {
        carve_corridor_v(y1, y2, x1);
        carve_corridor_h(x1, x2, y2);
    }
    
    a->connected = TRUE;
    b->connected = TRUE;
}

/* ---- Carve a horizontal corridor ---- */
static void carve_corridor_h(u8 x1, u8 x2, u8 y) {
    if (x1 > x2) { u8 t = x1; x1 = x2; x2 = t; }
    for (u8 x = x1; x <= x2; x++) {
        if (y > 0 && y < MAP_HEIGHT - 1 && x < MAP_WIDTH) {
            tilemap_set_tile(x, y, TILE_FLOOR);
            /* Also clear walls adjacent to corridor for 2-tile wide feel */
            if (y > 1 && tilemap_get_tile(x, y - 1) == TILE_WALL) {
                /* Keep wall on top */
            }
        }
    }
}

/* ---- Carve a vertical corridor ---- */
static void carve_corridor_v(u8 y1, u8 y2, u8 x) {
    if (y1 > y2) { u8 t = y1; y1 = y2; y2 = t; }
    for (u8 y = y1; y <= y2; y++) {
        if (x > 0 && x < MAP_WIDTH - 1 && y < MAP_HEIGHT) {
            tilemap_set_tile(x, y, TILE_FLOOR);
        }
    }
}

/* ---- Ensure all rooms are connected (DFS connect) ---- */
static void ensure_all_connected(void) {
    /* Mark room 0 as connected hub */
    s_dungeon.rooms[0].connected = TRUE;
    
    /* Iterate: find any unconnected room and connect it to nearest connected room */
    for (int iter = 0; iter < MAX_ROOMS; iter++) {
        bool all_connected = TRUE;
        for (u8 i = 0; i < s_dungeon.room_count; i++) {
            if (!s_dungeon.rooms[i].connected) {
                all_connected = FALSE;
                /* Find nearest connected room */
                s8 nearest = -1;
                s16 best_dist = 9999;
                for (u8 j = 0; j < s_dungeon.room_count; j++) {
                    if (s_dungeon.rooms[j].connected) {
                        s16 dx = s_dungeon.rooms[i].cx - s_dungeon.rooms[j].cx;
                        s16 dy = s_dungeon.rooms[i].cy - s_dungeon.rooms[j].cy;
                        s16 dist = ABS(dx) + ABS(dy);
                        if (dist < best_dist) {
                            best_dist = dist;
                            nearest = (s8)j;
                        }
                    }
                }
                if (nearest >= 0) {
                    connect_rooms(i, (u8)nearest);
                }
            }
        }
        if (all_connected) break;
    }
}

/**
 * Get current dungeon data.
 */
DungeonData* dungeon_get(void) {
    return &s_dungeon;
}

/**
 * Place all dungeon data onto the tilemap.
 */
void dungeon_place_tiles(void) {
    TileMap* map = tilemap_get();
    
    /* Fill entire map with walls */
    for (u8 y = 0; y < MAP_HEIGHT; y++) {
        for (u8 x = 0; x < MAP_WIDTH; x++) {
            map->tiles[y][x] = TILE_WALL;
        }
    }
    
    /* Carve rooms */
    for (u8 i = 0; i < s_dungeon.room_count; i++) {
        Room* room = &s_dungeon.rooms[i];
        
        for (u8 ry = 0; ry < room->h; ry++) {
            for (u8 rx = 0; rx < room->w; rx++) {
                u8 tx = room->x + rx;
                u8 ty = room->y + ry;
                
                /* Walls on edges, floor inside */
                if (rx == 0 || rx == room->w - 1 || ry == 0 || ry == room->h - 1) {
                    map->tiles[ty][tx] = TILE_WALL2;
                } else {
                    /* Alternate floor tiles for visual variety */
                    u8 floor_type = TILE_FLOOR;
                    if ((tx + ty) % 5 == 0) floor_type = TILE_FLOOR2;
                    if ((tx * ty) % 7 == 0) floor_type = TILE_FLOOR3;
                    map->tiles[ty][tx] = floor_type;
                }
            }
        }
        
        /* Place door opening (south wall, middle) */
        u8 door_x = room->cx;
        u8 door_y = room->y + room->h - 1;  /* Bottom wall */
        if (door_y < MAP_HEIGHT) {
            map->tiles[door_y][door_x] = TILE_DOOR;
        }
        
        /* Place special tiles */
        switch (room->type) {
            case ROOM_SPAWN:
                /* Mark with bones decoration */
                map->tiles[room->cy][room->cx] = TILE_BONES;
                break;
            case ROOM_BOSS:
                map->tiles[room->cy][room->cx] = TILE_ALTAR;
                map->tiles[room->cy][room->cx + 1] = TILE_STAIRS;
                s_dungeon.stair_x = room->cx + 1;
                s_dungeon.stair_y = room->cy;
                break;
            case ROOM_TREASURE:
                map->tiles[room->cy][room->cx] = TILE_CHEST;
                break;
            default:
                /* Random torch on walls */
                if (room->w > 4) {
                    u8 tx = room->x + (u8)rand_int(room->w - 2) + 1;
                    map->tiles[room->y][tx] = TILE_TORCH;
                }
                break;
        }
    }
    
    /* Carve corridors (they were already placed during generation) — 
     * Actually, corridors are carved onto the tilemap during connect_rooms().
     * But since dungeon_generate fills with walls after corridors, we need
     * to re-carve them. Let me fix the order: corridors should be carved 
     * AFTER wall fill. We re-do corridors here. */
    /* For now, the BSP already carved corridors correctly. The wall fill 
     * above overwrote them. We need to fix this. */
    /* 
     * FIX: We should carve corridors AFTER the wall fill.
     * Since corridors were carved during BSP before wall fill, they're gone.
     * Solution: re-carve all corridors now.
     */
    for (u8 i = 0; i < s_bsp_count; i++) {
        if (s_bsp[i].left >= 0 && s_bsp[i].right >= 0) {
            s8 lr = find_room_in_subtree((u8)s_bsp[i].left);
            s8 rr = find_room_in_subtree((u8)s_bsp[i].right);
            if (lr >= 0 && rr >= 0) {
                Room* a = &s_dungeon.rooms[lr];
                Room* b = &s_dungeon.rooms[rr];
                
                /* L-shaped corridor */
                if ((s_dungeon.seed + i) % 2 == 0) {
                    carve_corridor_h(a->cx, b->cx, a->cy);
                    carve_corridor_v(a->cy, b->cy, b->cx);
                } else {
                    carve_corridor_v(a->cy, b->cy, a->cx);
                    carve_corridor_h(a->cx, b->cx, b->cy);
                }
            }
        }
    }
}

/**
 * Get spawn position.
 */
void dungeon_get_spawn(u8* x, u8* y) {
    *x = s_dungeon.rooms[s_dungeon.spawn_room].cx;
    *y = s_dungeon.rooms[s_dungeon.spawn_room].cy;
}

/**
 * Check if position is inside any room.
 */
s8 dungeon_room_at(u8 x, u8 y) {
    for (u8 i = 0; i < s_dungeon.room_count; i++) {
        Room* r = &s_dungeon.rooms[i];
        if (x >= r->x && x < r->x + r->w && y >= r->y && y < r->y + r->h) {
            return (s8)i;
        }
    }
    return -1;
}

/**
 * Find nearest room.
 */
s8 dungeon_nearest_room(u8 x, u8 y) {
    s8 best = -1;
    s16 best_dist = 9999;
    
    for (u8 i = 0; i < s_dungeon.room_count; i++) {
        Room* r = &s_dungeon.rooms[i];
        s16 dx = (s16)x - (s16)r->cx;
        s16 dy = (s16)y - (s16)r->cy;
        s16 dist = ABS(dx) + ABS(dy);
        if (dist < best_dist) {
            best_dist = dist;
            best = (s8)i;
        }
    }
    return best;
}
