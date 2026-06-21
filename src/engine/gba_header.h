/**
 * gba_header.h — GBA Hardware Register Definitions
 * 
 * Minimal register definitions needed for Crypt of the Pixel.
 * Covers display, backgrounds, sprites, DMA, input, timers, and interrupts.
 */

#ifndef GBA_HEADER_H
#define GBA_HEADER_H

#include "gba_types.h"

/* ---- Memory-mapped I/O base ---- */
#define REG_BASE   0x04000000

/* ---- Display registers ---- */
#define REG_DISPCNT     (*(vu16*)0x04000000)  /* Display control */
#define REG_DISPSTAT    (*(vu16*)0x04000004)  /* Display status */
#define REG_VCOUNT      (*(vu16*)0x04000006)  /* Vertical counter */

/* Background control registers */
#define REG_BG0CNT      (*(vu16*)0x04000008)
#define REG_BG1CNT      (*(vu16*)0x0400000A)
#define REG_BG2CNT      (*(vu16*)0x0400000C)
#define REG_BG3CNT      (*(vu16*)0x0400000E)

/* Background scroll registers (x, y for each layer) */
#define REG_BG0HOFS     (*(vu16*)0x04000010)
#define REG_BG0VOFS     (*(vu16*)0x04000012)
#define REG_BG1HOFS     (*(vu16*)0x04000014)
#define REG_BG1VOFS     (*(vu16*)0x04000016)
#define REG_BG2HOFS     (*(vu16*)0x04000018)
#define REG_BG2VOFS     (*(vu16*)0x0400001A)
#define REG_BG3HOFS     (*(vu16*)0x0400001C)
#define REG_BG3VOFS     (*(vu16*)0x0400001E)

/* BG2/BG3 affine transformation parameters (Mode 1/2) */
#define REG_BG2X        (*(volatile s32*)0x04000028)
#define REG_BG2Y        (*(volatile s32*)0x0400002C)
#define REG_BG2PA       (*(vu16*)0x04000020)
#define REG_BG2PB       (*(vu16*)0x04000022)
#define REG_BG2PC       (*(vu16*)0x04000024)
#define REG_BG2PD       (*(vu16*)0x04000026)
#define REG_BG3X        (*(volatile s32*)0x04000038)
#define REG_BG3Y        (*(volatile s32*)0x0400003C)
#define REG_BG3PA       (*(vu16*)0x04000030)
#define REG_BG3PB       (*(vu16*)0x04000032)
#define REG_BG3PC       (*(vu16*)0x04000034)
#define REG_BG3PD       (*(vu16*)0x04000036)

/* Window registers */
#define REG_WIN0H       (*(vu16*)0x04000040)
#define REG_WIN1H       (*(vu16*)0x04000042)
#define REG_WIN0V       (*(vu16*)0x04000044)
#define REG_WIN1V       (*(vu16*)0x04000046)
#define REG_WININ       (*(vu16*)0x04000048)
#define REG_WINOUT      (*(vu16*)0x0400004A)

/* Mosaic register */
#define REG_MOSAIC      (*(vu16*)0x0400004C)

/* Color special effects */
#define REG_BLDCNT      (*(vu16*)0x04000050)
#define REG_BLDALPHA    (*(vu16*)0x04000052)
#define REG_BLDY        (*(vu16*)0x04000054)

/* ---- DMA registers ---- */
#define REG_DMA0SAD     (*(volatile void*)0x040000B0)
#define REG_DMA0DAD     (*(volatile void*)0x040000B4)
#define REG_DMA0CNT     (*(vu32*)0x040000B8)
#define REG_DMA1SAD     (*(volatile void*)0x040000BC)
#define REG_DMA1DAD     (*(volatile void*)0x040000C0)
#define REG_DMA1CNT     (*(vu32*)0x040000C4)
#define REG_DMA2SAD     (*(volatile void*)0x040000C8)
#define REG_DMA2DAD     (*(volatile void*)0x040000CC)
#define REG_DMA2CNT     (*(vu32*)0x040000D0)
#define REG_DMA3SAD     (*(volatile void*)0x040000D4)
#define REG_DMA3DAD     (*(volatile void*)0x040000D8)
#define REG_DMA3CNT     (*(vu32*)0x040000DC)

/* DMA control flags */
#define DMA_ENABLE      (1 << 31)
#define DMA_START_IF_HBLANK  (0 << 28)  /* Start immediately if dest < src, else HBlank */
#define DMA_START_VBLANK     (1 << 28)
#define DMA_START_HBLANK     (2 << 28)
#define DMA_START_SPECIAL    (3 << 28)
#define DMA_16              (0 << 26)  /* 16-bit transfers */
#define DMA_32              (1 << 26)  /* 32-bit transfers */
#define DMA_REPEAT          (1 << 25)  /* Repeat transfer */
#define DMA_SRC_INC         (0 << 23)  /* Increment source */
#define DMA_SRC_DEC         (1 << 23)  /* Decrement source */
#define DMA_SRC_FIXED       (2 << 23)  /* Fixed source */
#define DMA_SRC_RELOAD      (3 << 23)  /* Reload source at each repeat */
#define DMA_DST_INC         (0 << 21)  /* Increment destination */
#define DMA_DST_DEC         (1 << 21)  /* Decrement destination */
#define DMA_DST_FIXED       (2 << 21)  /* Fixed destination */
#define DMA_DST_RELOAD      (3 << 21)  /* Reload destination at each repeat */
#define DMA_IRQ             (1 << 20)  /* Enable IRQ on completion */

/* ---- Timer registers ---- */
#define REG_TM0CNT     (*(vu16*)0x04000100)
#define REG_TM0D       (*(vu16*)0x04000102)
#define REG_TM1CNT     (*(vu16*)0x04000104)
#define REG_TM1D       (*(vu16*)0x04000106)
#define REG_TM2CNT     (*(vu16*)0x04000108)
#define REG_TM2D       (*(vu16*)0x0400010A)
#define REG_TM3CNT     (*(vu16*)0x0400010C)
#define REG_TM3D       (*(vu16*)0x0400010E)

/* Timer control flags */
#define TM_ENABLE       (1 << 7)
#define TM_IRQ          (1 << 6)
#define TM_CASCADE      (1 << 2)
#define TM_FREQ_1       (0 << 0)   /* 1 cycle (16.78 MHz) */
#define TM_FREQ_64      (1 << 0)   /* 64 cycles */
#define TM_FREQ_256     (2 << 0)   /* 256 cycles */
#define TM_FREQ_1024    (3 << 0)   /* 1024 cycles (~16.38 kHz) */

/* ---- Input (keypad) ---- */
#define REG_KEYPAD      (*(vu16*)0x04000130)

/* Key bits (active low — 0 = pressed) */
#define KEY_A           (1 << 0)
#define KEY_B           (1 << 1)
#define KEY_SELECT      (1 << 2)
#define KEY_START       (1 << 3)
#define KEY_RIGHT       (1 << 4)
#define KEY_LEFT        (1 << 5)
#define KEY_UP          (1 << 6)
#define KEY_DOWN        (1 << 7)
#define KEY_R           (1 << 8)
#define KEY_L           (1 << 9)
#define KEY_NONE        0x03FF

/* ---- Interrupt registers ---- */
#define REG_IE          (*(vu16*)0x04002000)   /* Interrupt enable */
#define REG_IF          (*(vu16*)0x04002002)   /* Interrupt flags (ack) */
#define REG_IME         (*(vu16*)0x04002008)   /* Master interrupt enable */

/* Interrupt source flags */
#define IRQ_VBLANK      (1 << 0)
#define IRQ_HBLANK      (1 << 1)
#define IRQ_VCOUNT      (1 << 2)
#define IRQ_TIMER0      (1 << 3)
#define IRQ_TIMER1      (1 << 4)
#define IRQ_TIMER2      (1 << 5)
#define IRQ_TIMER3      (1 << 6)
#define IRQ_SERIAL      (1 << 7)
#define IRQ_DMA0        (1 << 8)
#define IRQ_DMA1        (1 << 9)
#define IRQ_DMA2        (1 << 10)
#define IRQ_DMA3        (1 << 11)
#define IRQ_KEYPAD      (1 << 12)
#define IRQ_GAMEPAK     (1 << 13)

/* ---- Display mode flags ---- */
#define MODE_0          0x00   /* 4 text BG layers */
#define MODE_1          0x01   /* 2 text BG + 1 affine BG */
#define MODE_2          0x02   /* 2 affine BG */
#define MODE_3          0x03   /* Bitmap (16-bit, 240x160) */
#define MODE_4          0x04   /* Bitmap (8-bit, 2 framebuffers) */
#define MODE_5          0x05   /* Bitmap (16-bit, 160x128) */

/* BG layer enable bits */
#define BG0_ENABLE      (1 << 8)
#define BG1_ENABLE      (1 << 9)
#define BG2_ENABLE      (1 << 10)
#define BG3_ENABLE      (1 << 11)
#define OBJ_ENABLE      (1 << 12)
#define WIN0_ENABLE     (1 << 13)
#define WIN1_ENABLE     (1 << 14)
#define OBJWIN_ENABLE   (1 << 15)

/* BG control register fields */
#define BG_PRIORITY(n)          ((n) & 0x3)
#define BG_TILEBLOCK(n)         (((n) & 0x3) << 2)
#define BG_MOSAIC               (1 << 6)
#define BG_8BPP                 (1 << 7)
#define BG_MAPBASE(n)           (((n) & 0x1F) << 8)
#define BG_WRAP                 (1 << 13)  /* Affine BG only */
#define BG_SIZE_256x256         (0 << 14)
#define BG_SIZE_512x256         (1 << 14)
#define BG_SIZE_256x512         (2 << 14)
#define BG_SIZE_512x512         (3 << 14)

/* Sprite attribute flags */
#define OBJ_SHAPE_SQUARE        (0 << 14)
#define OBJ_SHAPE_WIDE          (1 << 14)
#define OBJ_SHAPE_TALL          (2 << 14)

#define OBJ_SIZE_8              (0 << 12)
#define OBJ_SIZE_16             (1 << 12)
#define OBJ_SIZE_32             (2 << 12)
#define OBJ_SIZE_64             (3 << 12)

/* Sprite mode flags */
#define OBJ_NORMAL              (0 << 10)
#define OBJ_SEMI_TRANSPARENT    (1 << 10)
#define OBJ_WINDOW              (2 << 10)

#define OBJ_COLORS_16           (0 << 13)
#define OBJ_COLORS_256          (1 << 13)

/* ---- VRAM addresses ---- */
#define VRAM        ((u16*)0x06000000)
#define VRAM_BG     ((u16*)0x06000000)     /* BG tile data / tilemaps */
#define VRAM_OBJ    ((u16*)0x06010000)     /* Sprite tile data */
#define VRAM_AFF    ((u16*)0x06000000)     /* Affine BG tile data */
#define VRAM_MAP    ((u16*)0x06000000)     /* Screen base block 0 */

#define TILE_BASE_0     (0x0000)  /* Character base block 0 */
#define TILE_BASE_1     (0x4000)  /* Character base block 1 */
#define TILE_BASE_2     (0x8000)  /* Character base block 2 */
#define TILE_BASE_3     (0xC000)  /* Character base block 3 */

#define MAP_BASE_0      (0xF800)  /* Screen base block 31 (highest) */
#define MAP_BASE_1      (0xF000)  /* Screen base block 30 */
#define MAP_BASE_2      (0xE800)  /* Screen base block 29 */
#define MAP_BASE_3      (0xE000)  /* Screen base block 28 */

/* ---- OAM (Object Attribute Memory) ---- */
#define OAM            ((u32*)0x07000000)
#define OAM_ATTR       ((u16*)0x07000000)  /* Attribute 0-2 for each sprite */
#define OAM_AFF        ((u16*)0x07000200)  /* Affine parameter sets (16 entries, pa/pb/pc/pd) */

/* ---- Palette RAM ---- */
#define PAL_BG         ((u16*)0x05000000)  /* Background palette (256 colors) */
#define PAL_BG_MEM(n)  ((u16*)0x05000000 + (n))
#define PAL_OBJ        ((u16*)0x05000200)  /* Sprite palette (256 colors) */
#define PAL_OBJ_MEM(n) ((u16*)0x05000200 + (n))

/* ---- EWRAM / IWRAM ---- */
#define EWRAM          ((u8*)0x02000000)
#define IWRAM          ((u8*)0x03000000)

/* ---- SRAM (save data) ---- */
#define SRAM           ((u8*)0x0E000000)
#define SRAM_SIZE      32768  /* 32KB */

/* ---- ROM header structure ---- */
typedef struct {
    u32 entry_point;      /* ARM branch instruction to entry point */
    u8  logo[0x9C];       /* Nintendo logo (156 bytes) */
    char game_title[12];  /* Game title (uppercase ASCII) */
    char game_code[4];    /* Game code */
    char maker_code[2];   /* Maker code */
    u8  fixed_96;         /* Fixed value: 0x96 */
    u8  main_unit;        /* Main unit code */
    u8  device_type;      /* Device type */
    u8  reserved[7];      /* Reserved area */
    u8  version;           /* Software version */
    u8  complement;        /* Complement (header checksum) */
    u16 checksum;         /* Global checksum */
} GbaHeader;

/* ---- Display status flags ---- */
#define DISPSTAT_VBL    (1 << 0)
#define DISPSTAT_HBL    (1 << 1)
#define DISPSTAT_VCT    (1 << 2)
#define DISPSTAT_VBL_IRQ (1 << 3)
#define DISPSTAT_HBL_IRQ (1 << 4)
#define DISPSTAT_VCT_IRQ (1 << 5)

#endif /* GBA_HEADER_H */
