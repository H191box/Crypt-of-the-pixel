/**
 * gba_types.h — Fundamental type definitions for GBA development
 * 
 * Uses GBA naming conventions: u8, u16, u32, s8, s16, s32.
 * Also defines fixed-point types and common macros.
 */

#ifndef GBA_TYPES_H
#define GBA_TYPES_H

/* ---- Unsigned integer types ---- */
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;

/* ---- Signed integer types ---- */
typedef signed char        s8;
typedef signed short       s16;
typedef signed int         s32;

/* ---- Boolean type ---- */
typedef enum { FALSE = 0, TRUE = 1 } bool;

/* ---- Void pointer type alias ---- */
typedef void*              voidp;

/* ---- Fixed-point types ---- */
typedef s32  fixed;          /* 20.12 fixed-point: integer part = val >> 12 */
typedef u32  ufixed;         /* Unsigned 20.12 fixed-point */

/* 8.8 fixed-point (for affine transforms) */
typedef s16  fixed8_8;
typedef u16  ufixed8_8;

/* ---- Fixed-point macros ---- */
#define FIXED_SHIFT      12
#define FLOAT_TO_FIXED(x)   ((fixed)((x) * (1 << FIXED_SHIFT)))
#define FIXED_TO_INT(x)     ((s32)((x) >> FIXED_SHIFT))
#define FIXED_TO_FLOAT(x)   ((float)(x) / (float)(1 << FIXED_SHIFT))
#define INT_TO_FIXED(x)     ((fixed)((x) << FIXED_SHIFT))
#define FIXED_MUL(a, b)     (((fixed)(a) * (fixed)(b)) >> FIXED_SHIFT)
#define FIXED_DIV(a, b)     (((fixed)(a) << FIXED_SHIFT) / (fixed)(b))

/* 8.8 fixed-point macros (for BG affine params) */
#define F8_SHIFT         8
#define FLOAT_TO_F8(x)      ((fixed8_8)((x) * (1 << F8_SHIFT)))
#define F8_TO_INT(x)        ((s32)((x) >> F8_SHIFT))
#define INT_TO_F8(x)        ((fixed8_8)((x) << F8_SHIFT))
#define F8_MUL(a, b)        (((fixed8_8)(a) * (fixed8_8)(b)) >> F8_SHIFT)

/* ---- Volatile helpers ---- */
typedef volatile u8   vu8;
typedef volatile u16  vu16;
typedef volatile u32  vu32;
typedef volatile s8   vs8;
typedef volatile s16  vs16;
typedef volatile s32  vs32;

/* ---- Common utility macros ---- */
#define BIT(n)           (1 << (n))
#define BIT_SET(var, n)  ((var) |= BIT(n))
#define BIT_CLR(var, n)  ((var) &= ~BIT(n))
#define BIT_TST(var, n)  ((var) & BIT(n))

#define MIN(a, b)        ((a) < (b) ? (a) : (b))
#define MAX(a, b)        ((a) > (b) ? (a) : (b))
#define CLAMP(x, lo, hi) (((x) < (lo)) ? (lo) : ((x) > (hi)) ? (hi) : (x))
#define ABS(x)           ((x) < 0 ? -(x) : (x))

#define ARRAY_SIZE(arr)  (sizeof(arr) / sizeof((arr)[0]))
#define ALIGN(n, a)      (((n) + ((a) - 1)) & ~((a) - 1))

/* ---- Memory barriers for register writes ---- */
#define REG_WRITE(reg, val) do { (reg) = (val); (void)(reg); } while(0)

/* ---- Non-returning function marker ---- */
#define NORETURN  __attribute__((noreturn))

/* ---- Place function in IWRAM for speed ---- */
#define IWRAM_CODE __attribute__((section(".iwram")))

/* ---- Align data ---- */
#define ALIGN4  __attribute__((aligned(4)))

/* ---- Unused variable suppression ---- */
#define UNUSED(x)  ((void)(x))

#endif /* GBA_TYPES_H */
