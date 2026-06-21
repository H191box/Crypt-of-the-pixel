/**
 * combat.c — Turn-Based Combat Implementation
 * 
 * Combat is resolved on the dungeon map (not a separate screen).
 * When the player attacks or is attacked:
 * - Damage is calculated based on stats
 * - Results are shown in the message log
 * - If player dies → game over
 * - If enemy dies → XP + loot
 */

#include "gba_types.h"
#include "random.h"
#include "player.h"
#include "enemy.h"
#include "combat.h"

/* ---- Combat state ---- */
static CombatState s_state = COMBAT_IDLE;
static s8 s_target_id = -1;
static CombatResult s_result;

/**
 * Initialize combat system.
 */
void combat_init(void) {
    s_state = COMBAT_IDLE;
    s_target_id = -1;
    s_result.player_dmg_dealt = 0;
    s_result.player_dmg_taken = 0;
    s_result.enemy_killed = FALSE;
    s_result.player_dodged = FALSE;
    s_result.player_crit = FALSE;
    s_result.flee_success = FALSE;
}

/**
 * Start combat.
 */
void combat_start(s8 enemy_id) {
    if (enemy_id < 0) return;
    s_state = COMBAT_PLAYER_TURN;
    s_target_id = enemy_id;
    
    /* Reset result */
    s_result.player_dmg_dealt = 0;
    s_result.player_dmg_taken = 0;
    s_result.enemy_killed = FALSE;
    s_result.player_dodged = FALSE;
    s_result.player_crit = FALSE;
    s_result.flee_success = FALSE;
}

/**
 * End combat.
 */
void combat_end(void) {
    s_state = COMBAT_IDLE;
    s_target_id = -1;
}

/**
 * Get state.
 */
CombatState combat_get_state(void) {
    return s_state;
}

/**
 * Get target enemy.
 */
s8 combat_get_target(void) {
    return s_target_id;
}

/**
 * Get last result.
 */
CombatResult combat_get_result(void) {
    return s_result;
}

/**
 * Process player action.
 */
void combat_player_action(CombatAction action) {
    if (s_state != COMBAT_PLAYER_TURN) return;
    
    Player* pl = player_get();
    Enemy* e = enemy_get(s_target_id);
    if (!e) { combat_end(); return; }
    
    switch (action) {
        case ACTION_ATTACK: {
            s16 base = pl->attack;
            
            /* Weapon bonus */
            if (pl->weapon_id == 1) base += 3;
            else if (pl->weapon_id == 2) base += 2;
            else if (pl->weapon_id == 3) base += 1;
            else if (pl->weapon_id == 4) base += 5;
            
            s16 variance = (s16)rand_range(-2, 2);
            s16 dmg = base + variance - e->defense / 2;
            if (dmg < 1) dmg = 1;
            
            /* Thief crit */
            s_result.player_crit = FALSE;
            if (pl->p_class == CLASS_THIEF && rand_chance(15)) {
                dmg *= 2;
                s_result.player_crit = TRUE;
            }
            
            s16 remaining = enemy_damage(s_target_id, dmg);
            s_result.player_dmg_dealt = dmg;
            s_result.enemy_killed = (remaining < 0) ? TRUE : FALSE;
            break;
        }
        
        case ACTION_MAGIC: {
            if (pl->mp < 5) {
                s_result.player_dmg_dealt = -1;  /* Not enough MP */
                break;
            }
            s16 dmg = pl->magic * 2 + (s16)rand_range(-1, 3);
            if (dmg < 1) dmg = 1;
            pl->mp -= 5;
            
            s16 remaining = enemy_damage(s_target_id, dmg);
            s_result.player_dmg_dealt = dmg;
            s_result.enemy_killed = (remaining < 0) ? TRUE : FALSE;
            break;
        }
        
        case ACTION_ITEM: {
            /* Use first health potion in inventory */
            for (int i = 0; i < MAX_INVENTORY; i++) {
                if (pl->inventory[i].type == 10) {
                    player_use_item(i);
                    s_result.player_dmg_dealt = 0;
                    break;
                }
            }
            break;
        }
        
        case ACTION_FLEE: {
            /* Flee chance based on player speed vs enemy speed */
            u8 chance = (u8)CLAMP(pl->speed * 10 - e->speed * 5, 20, 80);
            if (rand_chance(chance)) {
                s_result.flee_success = TRUE;
                s_state = COMBAT_FLED;
                return;
            } else {
                s_result.flee_success = FALSE;
            }
            break;
        }
    }
    
    /* If enemy survived and combat continues, enemy gets a turn */
    if (!s_result.enemy_killed && s_state != COMBAT_FLED) {
        s_state = COMBAT_ENEMY_TURN;
        combat_enemy_action();
    } else {
        s_state = COMBAT_RESULT;
    }
}

/**
 * Enemy takes its turn.
 */
void combat_enemy_action(void) {
    Player* pl = player_get();
    Enemy* e = enemy_get(s_target_id);
    if (!e || !pl->alive) {
        s_state = COMBAT_RESULT;
        return;
    }
    
    /* Enemy attacks player */
    s16 base_dmg = e->attack;
    s16 variance = (s16)rand_range(-1, 2);
    s16 dmg = base_dmg + variance;
    if (dmg < 1) dmg = 1;
    
    /* Player evasion check */
    s_result.player_dodged = FALSE;
    if (rand_int(100) < pl->evasion) {
        s_result.player_dodged = TRUE;
        s_result.player_dmg_taken = 0;
    } else {
        s16 reduced = dmg - pl->defense / 2;
        if (reduced < 1) reduced = 1;
        s_result.player_dmg_taken = reduced;
        player_take_damage(reduced);
    }
    
    /* Check if player died */
    if (!pl->alive) {
        s_state = COMBAT_RESULT;
        return;
    }
    
    s_state = COMBAT_RESULT;
}

/**
 * Check if combat is active.
 */
bool combat_active(void) {
    return (s_state != COMBAT_IDLE) ? TRUE : FALSE;
}
