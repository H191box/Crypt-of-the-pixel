/**
 * item_data.h — Item Type Definitions
 * 
 * Complete table of all items in the game with their effects,
 * names, and properties.
 */

#ifndef ITEM_DATA_H
#define ITEM_DATA_H

#include "gba_types.h"
#include "items.h"

/* ---- Item definition (public, referenced by items.c) ---- */
/* Re-using ItemDef from items.h */

/* ---- Item definition table ---- */
extern const ItemDef g_item_defs[];
extern const u8 g_item_def_count;

#endif /* ITEM_DATA_H */
