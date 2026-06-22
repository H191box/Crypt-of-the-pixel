/**
 * enemy_data.c — Enemy Type Table
 * 
 * All enemy definitions in one place for easy tuning.
 */

#include "gba_types.h"
#include "enemy_data.h"

const EnemyDef g_enemy_defs[] = {
    /* id, name,               hp,  atk, def, spd, xp, gold, sprite, min_fl, behavior */
    {  0, "Slime",            8,    3,   1,   1,  5,    3,     0,      1,       0 },
    {  1, "Bat",              5,    4,   0,   2,  4,    2,     3,      1,       0 },
    {  2, "Skeleton",        15,    5,   3,   1, 10,    8,     6,      2,       1 },
    {  3, "Ghost",           12,    6,   1,   2, 12,   10,     9,      3,       1 },
    {  4, "Orc",             20,    7,   5,   1, 15,   12,    12,      4,       1 },
    { 10, "King Slime",      50,   10,   5,   1, 50,  100,    15,      1,       2 },
    { 11, "Bone Lord",       60,   12,   8,   1, 60,  120,    17,      4,       2 },
    { 12, "Demon Lord",      80,   15,  10,  2, 80,  150,    19,      7,       2 },
};

const u8 g_enemy_def_count = 8;
