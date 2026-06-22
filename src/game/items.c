/**
 * items.c — Item System Implementation
 */

#include "gba_types.h"
#include "player.h"
#include "items.h"
#include "item_data.h"

/* ---- Ground items ---- */
static GroundItem s_ground[MAX_GROUND_ITEMS];

/**
 * Initialize item system.
 */
void items_init(void) {
    for (int i = 0; i < MAX_GROUND_ITEMS; i++) {
        s_ground[i].active = FALSE;
        s_ground[i].item_type = 0;
        s_ground[i].gx = 0;
        s_ground[i].gy = 0;
        s_ground[i].sprite_slot = 0;
    }
}

/**
 * Get item definition.
 */
const ItemDef* item_get_def(u8 item_type) {
    if (item_type == 0 || item_type >= g_item_def_count) return NULL;
    return &g_item_defs[item_type];
}

/**
 * Use an item.
 */
bool item_use(u8 item_type) {
    const ItemDef* def = item_get_def(item_type);
    if (!def) return FALSE;
    
    Player* pl = player_get();
    
    switch (def->type) {
        case 3:  /* Consumable */
            if (item_type == ITEM_HEALTH_POTION_SMALL) {
                player_heal(def->effect);
                return TRUE;
            }
            if (item_type == ITEM_MANA_POTION_SMALL) {
                player_restore_mp(def->effect);
                return TRUE;
            }
            if (item_type == ITEM_HEALTH_POTION_LARGE) {
                player_heal(def->effect);
                return TRUE;
            }
            break;
        case 4:  /* Scroll */
            /* Scrolls deal damage to adjacent enemies */
            /* Handled separately in combat */
            return TRUE;
        case 5:  /* Gold */
            pl->gold += def->value;
            return TRUE;
        default:
            break;
    }
    
    return FALSE;
}

/**
 * Spawn item on ground.
 */
void item_spawn_ground(u8 item_type, u8 gx, u8 gy) {
    for (int i = 0; i < MAX_GROUND_ITEMS; i++) {
        if (!s_ground[i].active) {
            s_ground[i].active = TRUE;
            s_ground[i].item_type = item_type;
            s_ground[i].gx = gx;
            s_ground[i].gy = gy;
            s_ground[i].sprite_slot = 0;
            return;
        }
    }
}

/**
 * Pick up item at player position.
 */
u8 item_pickup_at_player(void) {
    Player* pl = player_get();
    
    for (int i = 0; i < MAX_GROUND_ITEMS; i++) {
        if (s_ground[i].active && 
            s_ground[i].gx == pl->gx && s_ground[i].gy == pl->gy) {
            u8 type = s_ground[i].item_type;
            
            /* Try to add to inventory */
            s8 slot = player_add_item(type);
            if (slot >= 0) {
                s_ground[i].active = FALSE;
                return type;
            }
            return 0;  /* Inventory full */
        }
    }
    return 0;  /* Nothing to pick up */
}

/**
 * Get ground item at position.
 */
GroundItem* item_get_ground(u8 gx, u8 gy) {
    for (int i = 0; i < MAX_GROUND_ITEMS; i++) {
        if (s_ground[i].active && s_ground[i].gx == gx && s_ground[i].gy == gy) {
            return &s_ground[i];
        }
    }
    return NULL;
}

/**
 * Get ground item list.
 */
GroundItem* items_get_ground_list(void) {
    return s_ground;
}

/**
 * Remove ground item.
 */
void item_remove_ground(u8 gx, u8 gy) {
    for (int i = 0; i < MAX_GROUND_ITEMS; i++) {
        if (s_ground[i].active && s_ground[i].gx == gx && s_ground[i].gy == gy) {
            s_ground[i].active = FALSE;
            return;
        }
    }
}
