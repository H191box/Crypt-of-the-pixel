/**
 * random.h — Fast Seeded Random Number Generator
 * 
 * Uses a xorshift32 algorithm: fast, tiny state (4 bytes), 
 * reasonable quality for game purposes. Not cryptographically
 * secure — perfect for procedural dungeon generation.
 */

#ifndef RANDOM_H
#define RANDOM_H

#include "gba_types.h"

/* ---- Functions ---- */

/**
 * Seed the random number generator.
 * @param seed  Any u32 value. Same seed = same sequence.
 */
void rand_seed(u32 seed);

/**
 * Get the current seed value (useful for save/load).
 * @return Current RNG state.
 */
u32 rand_get_seed(void);

/**
 * Generate a random u32.
 * @return A pseudo-random 32-bit value.
 */
u32 rand_next(void);

/**
 * Generate a random integer in [0, max).
 * @param max  Upper bound (exclusive).
 * @return Random value in range [0, max).
 */
u32 rand_int(u32 max);

/**
 * Generate a random integer in [min, max].
 * @param min  Lower bound (inclusive).
 * @param max  Upper bound (inclusive).
 * @return Random value in range [min, max].
 */
s32 rand_range(s32 min, s32 max);

/**
 * Generate a random boolean with given probability.
 * @param chance  Probability out of 100 (0-100).
 * @return TRUE with 'chance'% probability.
 */
bool rand_chance(u8 chance);

/**
 * Pick a random index from an array of weights.
 * @param weights  Array of u16 weights.
 * @param count    Number of entries in the array.
 * @return Index into the array.
 */
s32 rand_weighted(const u16* weights, u8 count);

#endif /* RANDOM_H */
