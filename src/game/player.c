/**
 * player.c — Player Character Implementation
 * 
 * Handles movement, stats, inventory, and leveling.
 * Player moves one tile per turn (turn-based roguelike).
 */

#include "gba_types.h"
#include "random.h"
#include "tilemap.h"
#include "iso_math.h"
#include "enemy.h"
#include "player.h"

/* ---- Player instance ---- */
static Player s_player;

/* ---- Class base stats ---- */
typedef struct {
    s16 hp;
    s16 mp;
    u8 atk;
    u8 def;
    u8 mag;
    u8 spd;
    u8 eva;
} ClassStats;

static const ClassStats s_class_stats[3] = {
    /* WARRIOR */ { 30,  5,  8,  6,  2,  4,  5 },
    /* MAGE    */ { 18, 20,  4,  3,  9,  5,  7 },
    /* THIEF   */ { 22, 10,  6,  4,  3,  8, 12 },
};

/* ---- Level-up stat gains ---- */
static const ClassStats s_level_gains[3] = {
    /* WARRIOR */ {  6,  2,  2,  2,  1,  1,  1 },
    /* MAGE    */ {  3,  5,  1,  1,  3,  1,  1 },
    /* THIEF   */ {  4,  3,  2,  1,  1,  2,  2 },
};

/**
 * Initialize player with given class.
 */
void player_init(PlayerClass p_class) {
    ClassStats base = s_class_stats[p_class];
    
    s_player.gx = 0;
    s_player.gy = 0;
    s_player.hp = base.hp;
    s_player.max_hp = base.hp;
    s_player.mp = base.mp;
    s_player.max_mp = base.mp;
    s_player.attack = base.atk;
    s_player.defense = base.def;
    s_player.magic = base.mag;
    s_player.speed = base.spd;
    s_player.evasion = base.eva;
    s_player.level = 1;
    s_player.xp = 0;
    s_player.xp_to_next = 50;
    s_player.gold = 0;
    s_player.p_class = p_class;
    s_player.facing = ISO_DIR_S;
    s_player.alive = TRUE;
    s_player.acted = FALSE;
    s_player.weapon_id = 0;
    s_player.armor_id = 0;
    
    /* Clear inventory */
    for (int i = 0; i < MAX_INVENTORY; i++) {
        s_player.inventory[i].type = 0;
        s_player.inventory[i].count = 0;
    }
    
    /* Starting items based on class */
    switch (p_class) {
        case CLASS_WARRIOR:
            player_add_item(1);  /* Short sword */
            player_add_item(5);  /* Leather armor */
            player_add_item(10); /* 2 health potions */
            player_add_item(10);
            break;
        case CLASS_MAGE:
            player_add_item(3);  /* Magic staff */
            player_add_item(11); /* 2 mana potions */
            player_add_item(11);
            player_add_item(10); /* 1 health potion */
            break;
        case CLASS_THIEF:
            player_add_item(2);  /* Dagger */
            player_add_item(10); /* 2 health potions */
            player_add_item(10);
            break;
    }
}

/**
 * Get player data.
 */
Player* player_get(void) {
    return &s_player;
}

/**
 * Move player by (dx, dy).
 */
bool player_move(s8 dx, s8 dy) {
    u8 nx = (u8)((s16)s_player.gx + dx);
    u8 ny = (u8)((s16)s_player.gy + dy);
    
    /* Bounds check */
    if (nx >= MAP_WIDTH || ny >= MAP_HEIGHT) return FALSE;
    
    /* Check tile walkability */
    if (!tilemap_is_walkable(nx, ny)) return FALSE;
    
    /* Check for enemy at destination */
    if (enemy_at(nx, ny) >= 0) return FALSE;  /* Can't walk into enemies */
    
    /* Move */
    s_player.gx = nx;
    s_player.gy = ny;
    
    /* Update facing direction */
    if (dx < 0) s_player.facing = ISO_DIR_W;
    else if (dx > 0) s_player.facing = ISO_DIR_E;
    else if (dy < 0) s_player.facing = ISO_DIR_N;
    else if (dy > 0) s_player.facing = ISO_DIR_S;
    
    s_player.acted = TRUE;
    return TRUE;
}

/**
 * Player attacks adjacent enemy.
 * Checks all 4 adjacent tiles for enemies, attacks the first one found.
 */
s16 player_attack(void) {
    s8 dirs[4][2] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };
    
    for (int d = 0; d < 4; d++) {
        u8 tx = (u8)((s16)s_player.gx + dirs[d][0]);
        u8 ty = (u8)((s16)s_player.gy + dirs[d][1]);
        
        s8 eid = enemy_at(tx, ty);
        if (eid >= 0) {
            /* Calculate damage: ATK + weapon_bonus - enemy_DEF/2 + variance */
            s16 base_dmg = s_player.attack;
            /* Weapon bonus */
            if (s_player.weapon_id == 1) base_dmg += 3;   /* Short sword */
            else if (s_player.weapon_id == 2) base_dmg += 2;  /* Dagger */
            else if (s_player.weapon_id == 3) base_dmg += 1;  /* Magic staff */
            else if (s_player.weapon_id == 4) base_dmg += 5;  /* Long sword */
            
            s16 variance = (s16)rand_range(-2, 2);
            s16 total = base_dmg + variance;
            if (total < 1) total = 1;
            
            /* Crit chance for thief */
            if (s_player.p_class == CLASS_THIEF && rand_chance(15)) {
                total *= 2;  /* Double damage crit */
            }
            
            enemy_damage(eid, total);
            s_player.acted = TRUE;
            return total;
        }
    }
    
    return -1;  /* No enemy adjacent */
}

/**
 * Player casts magic.
 */
s16 player_cast_magic(u8 spell_id) {
    if (s_player.mp < 5) return -1;  /* Not enough MP */
    
    /* Find adjacent enemy for target */
    s8 dirs[4][2] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };
    for (int d = 0; d < 4; d++) {
        u8 tx = (u8)((s16)s_player.gx + dirs[d][0]);
        u8 ty = (u8)((s16)s_player.gy + dirs[d][1]);
        
        s8 eid = enemy_at(tx, ty);
        if (eid >= 0) {
            s16 dmg = s_player.magic * 2 + (s16)rand_range(-1, 3);
            if (dmg < 1) dmg = 1;
            
            s_player.mp -= 5;
            enemy_damage(eid, dmg);
            s_player.acted = TRUE;
            return dmg;
        }
    }
    
    return -1;  /* No target */
}

/**
 * Player takes damage.
 */
s16 player_take_damage(s16 amount) {
    /* Apply defense */
    s16 reduced = amount - s_player.defense / 2;
    if (reduced < 1) reduced = 1;
    
    /* Evasion check */
    if (rand_int(100) < s_player.evasion) {
        reduced = 0;  /* Dodged! */
    }
    
    s_player.hp -= reduced;
    if (s_player.hp <= 0) {
        s_player.hp = 0;
        s_player.alive = FALSE;
    }
    
    return reduced;
}

/**
 * Player uses item from inventory slot.
 */
bool player_use_item(u8 slot) {
    if (slot >= MAX_INVENTORY) return FALSE;
    InvSlot* item = &s_player.inventory[slot];
    if (item->type == 0 || item->count == 0) return FALSE;
    
    switch (item->type) {
        case 10:  /* Health potion */
            player_heal(15);
            break;
        case 11:  /* Mana potion */
            player_restore_mp(10);
            break;
        default:
            return FALSE;  /* Not a usable item */
    }
    
    item->count--;
    if (item->count == 0) item->type = 0;
    return TRUE;
}

/**
 * Add item to inventory.
 */
s8 player_add_item(u8 item_type) {
    /* Try to stack with existing */
    for (int i = 0; i < MAX_INVENTORY; i++) {
        if (s_player.inventory[i].type == item_type && 
            s_player.inventory[i].count < 99) {
            s_player.inventory[i].count++;
            return i;
        }
    }
    
    /* Find empty slot */
    for (int i = 0; i < MAX_INVENTORY; i++) {
        if (s_player.inventory[i].type == 0) {
            s_player.inventory[i].type = item_type;
            s_player.inventory[i].count = 1;
            return i;
        }
    }
    
    return -1;  /* Inventory full */
}

/**
 * Check if player has an item type.
 */
bool player_has_item(u8 item_type) {
    for (int i = 0; i < MAX_INVENTORY; i++) {
        if (s_player.inventory[i].type == item_type && s_player.inventory[i].count > 0) {
            return TRUE;
        }
    }
    return FALSE;
}

/**
 * Remove one instance of an item.
 */
void player_remove_item(u8 item_type) {
    for (int i = 0; i < MAX_INVENTORY; i++) {
        if (s_player.inventory[i].type == item_type) {
            s_player.inventory[i].count--;
            if (s_player.inventory[i].count == 0) {
                s_player.inventory[i].type = 0;
            }
            return;
        }
    }
}

/**
 * Gain XP and check for level up.
 */
void player_gain_xp(u16 amount) {
    s_player.xp += amount;
    while (s_player.xp >= s_player.xp_to_next) {
        s_player.xp -= s_player.xp_to_next;
        player_level_up();
    }
}

/**
 * Level up — increase stats based on class.
 */
void player_level_up(void) {
    s_player.level++;
    s_player.xp_to_next = (u16)(s_player.xp_to_next * 15 / 10);  /* +50% per level */
    
    ClassStats gains = s_level_gains[s_player.p_class];
    s_player.max_hp += gains.hp;
    s_player.max_mp += gains.mp;
    s_player.attack += gains.atk;
    s_player.defense += gains.def;
    s_player.magic += gains.mag;
    s_player.speed += gains.spd;
    s_player.evasion += gains.eva;
    
    /* Fully heal on level up */
    s_player.hp = s_player.max_hp;
    s_player.mp = s_player.max_mp;
}

/**
 * Heal player.
 */
void player_heal(s16 amount) {
    s_player.hp += amount;
    if (s_player.hp > s_player.max_hp) {
        s_player.hp = s_player.max_hp;
    }
}

/**
 * Restore MP.
 */
void player_restore_mp(s16 amount) {
    s_player.mp += amount;
    if (s_player.mp > s_player.max_mp) {
        s_player.mp = s_player.max_mp;
    }
}

/**
 * Check adjacency.
 */
bool player_adjacent_to(u8 x, u8 y) {
    s16 dx = ABS((s16)s_player.gx - (s16)x);
    s16 dy = ABS((s16)s_player.gy - (s16)y);
    return (dx + dy == 1) ? TRUE : FALSE;
}

/**
 * New turn — reset acted flag.
 */
void player_new_turn(void) {
    s_player.acted = FALSE;
}
