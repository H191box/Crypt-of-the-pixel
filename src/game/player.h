/**
 * player.h — Player Character
 * 
 * Manages player state: position, stats, inventory, class.
 * Turn-based movement on the isometric grid.
 */

#ifndef PLAYER_H
#define PLAYER_H

#include "gba_types.h"

/* ---- Player classes ---- */
typedef enum {
    CLASS_WARRIOR = 0,   /* High HP, high ATK, medium DEF */
    CLASS_MAGE    = 1,   /* Low HP, high magic, medium ATK */
    CLASS_THIEF   = 2    /* Medium HP, high evasion, high crit */
} PlayerClass;

/* ---- Inventory slots ---- */
#define MAX_INVENTORY  16

/* ---- Item slot ---- */
typedef struct {
    u8 type;       /* Item type ID */
    u8 count;      /* Stack count */
} InvSlot;

/* ---- Player data ---- */
typedef struct {
    /* Position (grid coords) */
    u8 gx;
    u8 gy;
    
    /* Stats */
    s16 hp;
    s16 max_hp;
    s16 mp;
    s16 max_mp;
    u8 attack;
    u8 defense;
    u8 magic;
    u8 speed;
    u8 evasion;
    
    /* Progression */
    u8 level;
    u16 xp;
    u16 xp_to_next;
    u16 gold;
    
    /* Class */
    u8 p_class;
    
    /* Inventory */
    InvSlot inventory[MAX_INVENTORY];
    
    /* Equipment */
    u8 weapon_id;     /* Item ID of equipped weapon, 0 = none */
    u8 armor_id;      /* Item ID of equipped armor, 0 = none */
    
    /* State */
    u8 facing;        /* IsoDirection */
    bool alive;
    bool acted;       /* Has the player taken their turn this round? */
} Player;

/* ---- Functions ---- */

/**
 * Initialize the player.
 * @param p_class  Player class (warrior, mage, thief)
 */
void player_init(PlayerClass p_class);

/**
 * Get pointer to player data.
 */
Player* player_get(void);

/**
 * Move the player in a direction.
 * @param dx  Grid X delta (-1, 0, +1)
 * @param dy  Grid Y delta (-1, 0, +1)
 * @return TRUE if move was successful.
 */
bool player_move(s8 dx, s8 dy);

/**
 * Player attacks an adjacent enemy.
 * @return Damage dealt, or -1 if no enemy adjacent.
 */
s16 player_attack(void);

/**
 * Player uses magic.
 * @param spell_id  Magic spell to cast.
 * @return Damage dealt, or -1 if failed.
 */
s16 player_cast_magic(u8 spell_id);

/**
 * Player takes damage.
 * @param amount  Damage amount.
 * @return HP remaining.
 */
s16 player_take_damage(s16 amount);

/**
 * Player uses an item from inventory.
 * @param slot  Inventory slot index.
 * @return TRUE if item was used successfully.
 */
bool player_use_item(u8 slot);

/**
 * Add an item to inventory.
 * @param item_type  Item type ID.
 * @return Inventory slot where item was placed, or -1 if full.
 */
s8 player_add_item(u8 item_type);

/**
 * Check if player has a specific item.
 */
bool player_has_item(u8 item_type);

/**
 * Remove one instance of an item from inventory.
 */
void player_remove_item(u8 item_type);

/**
 * Gain XP. Check for level up.
 * @param amount  XP gained.
 */
void player_gain_xp(u16 amount);

/**
 * Level up — increase stats.
 */
void player_level_up(void);

/**
 * Heal player.
 */
void player_heal(s16 amount);

/**
 * Restore MP.
 */
void player_restore_mp(s16 amount);

/**
 * Check if player is adjacent to position (x,y).
 */
bool player_adjacent_to(u8 x, u8 y);

/**
 * Reset the "acted" flag (for new turn).
 */
void player_new_turn(void);

#endif /* PLAYER_H */
