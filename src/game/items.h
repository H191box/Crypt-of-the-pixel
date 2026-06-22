/**
 * items.h — Item System
 * 
 * Manages item definitions, usage effects, and ground pickups.
 * Items are identified by type ID. Full data in item_data.c.
 */

#ifndef ITEMS_H
#define ITEMS_H

#include "gba_types.h"

/* ---- Item type IDs ---- */
typedef enum {
    ITEM_NONE         = 0,
    /* Weapons */
    ITEM_SHORT_SWORD  = 1,
    ITEM_DAGGER       = 2,
    ITEM_MAGIC_STAFF  = 3,
    ITEM_LONG_SWORD   = 4,
    /* Armor */
    ITEM_LEATHER_ARMOR = 5,
    ITEM_CHAIN_MAIL    = 6,
    ITEM_PLATE_ARMOR   = 7,
    /* Consumables */
    ITEM_HEALTH_POTION_SMALL = 10,
    ITEM_MANA_POTION_SMALL  = 11,
    ITEM_HEALTH_POTION_LARGE = 12,
    /* Special */
    ITEM_KEY           = 20,
    ITEM_SCROLL_FIRE   = 21,
    ITEM_SCROLL_ICE    = 22,
    ITEM_GOLD_SMALL    = 30,
    ITEM_GOLD_LARGE    = 31
} ItemType;

/* ---- Item definition ---- */
typedef struct {
    u8 id;
    const char* name;
    u8 type;        /* 0=none, 1=weapon, 2=armor, 3=consumable, 4=scroll, 5=gold */
    s16 effect;     /* Effect value (heal amount, damage, etc.) */
    u16 value;      /* Gold value for selling */
    u8 sprite_id;  /* Sprite tile index for ground display */
    bool stackable; /* Can multiple stack in one slot? */
} ItemDef;

/* ---- Ground item ---- */
typedef struct {
    bool active;
    u8 item_type;
    u8 gx;
    u8 gy;
    u8 sprite_slot;  /* Sprite manager slot for rendering */
} GroundItem;

/* ---- Constants ---- */
#define MAX_GROUND_ITEMS  16

/* ---- Functions ---- */

/**
 * Initialize item system.
 */
void items_init(void);

/**
 * Get item definition by type ID.
 */
const ItemDef* item_get_def(u8 item_type);

/**
 * Use an item (apply its effect to the player).
 * @param item_type  Item type ID
 * @return TRUE if successfully used.
 */
bool item_use(u8 item_type);

/**
 * Spawn an item on the ground at grid position.
 */
void item_spawn_ground(u8 item_type, u8 gx, u8 gy);

/**
 * Pick up ground item at player's position.
 * @return Item type picked up, or 0 if nothing.
 */
u8 item_pickup_at_player(void);

/**
 * Get ground item at position.
 */
GroundItem* item_get_ground(u8 gx, u8 gy);

/**
 * Get all active ground items.
 */
GroundItem* items_get_ground_list(void);

/**
 * Remove a ground item.
 */
void item_remove_ground(u8 gx, u8 gy);

#endif /* ITEMS_H */
