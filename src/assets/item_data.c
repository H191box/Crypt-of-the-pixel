/**
 * item_data.c — Item Type Table
 * 
 * All item definitions. The g_item_defs array is indexed by item type ID.
 * Unused IDs (e.g., 8, 9, etc.) have zero-filled entries.
 */

#include "gba_types.h"
#include "item_data.h"
#include "items.h"

const ItemDef g_item_defs[] = {
    /* id, name,             type, effect, value, sprite, stackable */
    {  0, "None",              0,      0,     0,      0,   FALSE },
    
    /* Weapons (type=1) */
    {  1, "Short Sword",       1,      0,    25,      2,   FALSE },
    {  2, "Dagger",            1,      0,    15,      2,   FALSE },
    {  3, "Magic Staff",       1,      0,    30,      2,   FALSE },
    {  4, "Long Sword",        1,      0,    60,      2,   FALSE },
    
    /* Armor (type=2) */
    {  5, "Leather Armor",     2,      0,    20,      2,   FALSE },
    {  6, "Chain Mail",        2,      0,    45,      2,   FALSE },
    {  7, "Plate Armor",       2,      0,    80,      2,   FALSE },
    
    /* 8-9: unused */
    {  8, "",                  0,      0,     0,      0,   FALSE },
    {  9, "",                  0,      0,     0,      0,   FALSE },
    
    /* Consumables (type=3) */
    { 10, "HP Potion S",       3,     15,    10,      3,   TRUE  },
    { 11, "MP Potion S",       3,     10,    12,      3,   TRUE  },
    { 12, "HP Potion L",       3,     40,    30,      3,   TRUE  },
    
    /* 13-19: unused */
    { 13, "",                  0,      0,     0,      0,   FALSE },
    { 14, "",                  0,      0,     0,      0,   FALSE },
    { 15, "",                  0,      0,     0,      0,   FALSE },
    { 16, "",                  0,      0,     0,      0,   FALSE },
    { 17, "",                  0,      0,     0,      0,   FALSE },
    { 18, "",                  0,      0,     0,      0,   FALSE },
    { 19, "",                  0,      0,     0,      0,   FALSE },
    
    /* Scrolls (type=4) */
    { 20, "Key",               5,      0,     5,      4,   TRUE  },
    { 21, "Fire Scroll",       4,     20,    35,      4,   TRUE  },
    { 22, "Ice Scroll",        4,     15,    30,      4,   TRUE  },
    
    /* 23-29: unused */
    { 23, "",                  0,      0,     0,      0,   FALSE },
    { 24, "",                  0,      0,     0,      0,   FALSE },
    { 25, "",                  0,      0,     0,      0,   FALSE },
    { 26, "",                  0,      0,     0,      0,   FALSE },
    { 27, "",                  0,      0,     0,      0,   FALSE },
    { 28, "",                  0,      0,     0,      0,   FALSE },
    { 29, "",                  0,      0,     0,      0,   FALSE },
    
    /* Gold (type=5) */
    { 30, "Gold (small)",       5,      0,    10,      5,   TRUE  },
    { 31, "Gold (large)",       5,      0,    50,      5,   TRUE  },
};

const u8 g_item_def_count = 32;  /* Total entries in the array */
