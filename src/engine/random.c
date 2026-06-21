/**
 * random.c — xorshift32 Random Number Generator
 * 
 * xorshift32 is one of the fastest PRNGs available:
 * - State: single u32 (4 bytes)
 * - Speed: 3 XOR + 3 shift operations per number
 * - Period: 2^32 - 1 (before repeating)
 * - Quality: Passes many statistical tests, fine for game use
 * 
 * We never allow state = 0 (xorshift degenerates to zero).
 */

#include "gba_types.h"
#include "random.h"

/* ---- RNG state ---- */
static u32 s_state = 1;  /* Must never be 0 */

/**
 * Seed the RNG.
 */
void rand_seed(u32 seed) {
    /* xorshift can't start with 0 — map 0 to 1 */
    s_state = seed ? seed : 1;
}

/**
 * Get current seed for save/load.
 */
u32 rand_get_seed(void) {
    return s_state;
}

/**
 * Generate next random u32 using xorshift32.
 */
u32 rand_next(void) {
    u32 x = s_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    s_state = x;
    return x;
}

/**
 * Random integer in [0, max).
 * Uses modular reduction — slight bias for non-power-of-2 max,
 * but acceptable for game purposes.
 */
u32 rand_int(u32 max) {
    if (max == 0) return 0;
    return rand_next() % max;
}

/**
 * Random integer in [min, max].
 */
s32 rand_range(s32 min, s32 max) {
    if (min >= max) return min;
    s32 range = max - min + 1;
    return min + (s32)(rand_next() % (u32)range);
}

/**
 * Random boolean with given percent chance.
 */
bool rand_chance(u8 chance) {
    if (chance >= 100) return TRUE;
    if (chance == 0) return FALSE;
    return (rand_next() % 100) < chance ? TRUE : FALSE;
}

/**
 * Pick from weighted options.
 * Sums all weights, picks a random value in [0, total), 
 * walks through weights to find which index it falls in.
 */
s32 rand_weighted(const u16* weights, u8 count) {
    if (count == 0) return 0;
    
    /* Calculate total weight */
    u32 total = 0;
    for (u8 i = 0; i < count; i++) {
        total += weights[i];
    }
    
    if (total == 0) return 0;
    
    u32 pick = rand_next() % total;
    u32 accum = 0;
    
    for (u8 i = 0; i < count; i++) {
        accum += weights[i];
        if (pick < accum) return i;
    }
    
    return count - 1;
}
