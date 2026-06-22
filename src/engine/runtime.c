/**
 * runtime.c — ARM EABI runtime support for GBA bare-metal
 *
 * Provides software implementations of division and memory operations
 * required when linking with -nostdlib.
 */

#include "../engine/gba_types.h"

/* ---- Unsigned division ---- */
u32 __aeabi_uidiv(u32 numerator, u32 denominator) {
    if (denominator == 0) return 0;
    u32 quot = 0;
    u32 bit = 1;
    
    /* Align denominator with numerator */
    while (bit && denominator <= numerator) {
        denominator <<= 1;
        bit <<= 1;
    }
    denominator >>= 1;
    bit >>= 1;
    
    /* Subtract and accumulate quotient */
    while (bit) {
        if (numerator >= denominator) {
            numerator -= denominator;
            quot |= bit;
        }
        denominator >>= 1;
        bit >>= 1;
    }
    return quot;
}

/* ---- Unsigned divmod (returns struct: r0=quot, r1=rem) ---- */
typedef struct { u32 quot; u32 rem; } udiv_t;

udiv_t __aeabi_uidivmod(u32 num, u32 den) {
    udiv_t result;
    result.quot = __aeabi_uidiv(num, den);
    result.rem = num - result.quot * den;
    return result;
}

/* ---- Signed division ---- */
s32 __aeabi_idiv(s32 numerator, s32 denominator) {
    int neg = 0;
    if (numerator < 0) { numerator = -numerator; neg = !neg; }
    if (denominator < 0) { denominator = -denominator; neg = !neg; }
    s32 result = (s32)__aeabi_uidiv((u32)numerator, (u32)denominator);
    return neg ? -result : result;
}

/* ---- Signed divmod ---- */
typedef struct { s32 quot; s32 rem; } sdiv_t;

sdiv_t __aeabi_idivmod(s32 num, s32 den) {
    sdiv_t result;
    result.quot = __aeabi_idiv(num, den);
    result.rem = num - result.quot * den;
    return result;
}

/* ---- Memory operations ---- */
void* memcpy(void* dest, const void* src, u32 n) {
    u8* d = (u8*)dest;
    const u8* s = (const u8*)src;
    while (n--) *d++ = *s++;
    return dest;
}

void* memset(void* dest, s32 val, u32 n) {
    u8* d = (u8*)dest;
    while (n--) *d++ = (u8)val;
    return dest;
}

void* memmove(void* dest, const void* src, u32 n) {
    u8* d = (u8*)dest;
    const u8* s = (const u8*)src;
    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        d += n;
        s += n;
        while (n--) *--d = *--s;
    }
    return dest;
}
