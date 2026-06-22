/**
 * system.c — GBA Hardware Initialization and VBlank Management
 * 
 * Sets up Mode 0 (4 text BG layers), sprites, palettes, and
 * a VBlank interrupt handler. This is the first subsystem
 * that must be initialized in the game.
 */

#include "gba_header.h"
#include "gba_types.h"
#include "system.h"

/* ---- Global frame counter ---- */
u32 g_frame_count = 0;

/* ---- VBlank callback ---- */
static VBlankCallback s_vblank_cb = NULL;

/* ---- Assembly trampoline for VBlank ISR ---- */
/* The ARM7TDMI requires ISRs to be in ARM mode, not Thumb.   */
/* We provide a tiny assembly stub that switches to Thumb.     */
/* For simplicity in this project, we handle the ISR in C by   */
/* reading/clearing REG_IF directly.                          */
/* The linker script must place the vector table at 0x03000000 */

/* ---- ROM header at 0x08000000 ---- */
/* The GBA BIOS reads the entry point from offset 0 and jumps there. */
/* We put a branch instruction there. The actual GBA header is at    */
/* 0x08000000 + 0xC0 (the GBA reads 0x200 bytes from ROM start).    */

/* ISR: called when VBlank fires. Must be in ARM mode or use proper prologue. */
/* We handle it in C by disabling interworking concerns — the compiler */
/* will generate the right entry for a Thumb function if linked properly. */

/**
 * VBlank interrupt service routine.
 * Reads/clears interrupt flags and calls the user callback.
 * Placed in IWRAM for fast response.
 */
void irq_handler(void) {
    u16 irq_flags = REG_IF;
    
    /* Acknowledge all handled interrupts */
    REG_IF = irq_flags;
    
    /* Handle VBlank */
    if (irq_flags & IRQ_VBLANK) {
        g_frame_count++;
        if (s_vblank_cb) {
            s_vblank_cb();
        }
    }
}

/**
 * Initialize the GBA hardware for our isometric dungeon crawler.
 */
void sys_init(void) {
    /* ---- Set up interrupt vector table in IWRAM ---- */
    /* The GBA reads the ISR address from 0x03000008 (BIOS uses 0x03000000-0x0300000C) */
    /* We write our handler address there */
    volatile u32* isr_vector = (volatile u32*)0x03000008;
    *isr_vector = (u32)&irq_handler;
    
    /* ---- Initialize display ---- */
    /* Mode 0: 4 text BG layers, all enabled + sprites */
    u16 disp_mode = MODE_0;
    REG_DISPCNT = disp_mode | BG0_ENABLE | BG1_ENABLE | BG2_ENABLE | BG3_ENABLE | OBJ_ENABLE;
    
    /* ---- Configure BG layers ---- */
    /* BG0 — Floor tiles: priority 0, char block 0, screen block 28 (0xE000) */
    REG_BG0CNT = BG_PRIORITY(0) | BG_TILEBLOCK(0) | BG_MAPBASE(28);
    
    /* BG1 — Wall tiles: priority 1, char block 1, screen block 30 (0xF000) */
    REG_BG1CNT = BG_PRIORITY(1) | BG_TILEBLOCK(1) | BG_MAPBASE(30);
    
    /* BG2 — Mini-map: priority 2, char block 0, screen block 29 (0xE800) */
    REG_BG2CNT = BG_PRIORITY(2) | BG_TILEBLOCK(0) | BG_MAPBASE(29);
    
    /* BG3 — HUD/text: priority 3, char block 0, screen block 31 (0xF800) */
    REG_BG3CNT = BG_PRIORITY(3) | BG_TILEBLOCK(0) | BG_MAPBASE(31);
    
    /* ---- Clear all background scroll positions ---- */
    REG_BG0HOFS = 0; REG_BG0VOFS = 0;
    REG_BG1HOFS = 0; REG_BG1VOFS = 0;
    REG_BG2HOFS = 0; REG_BG2VOFS = 0;
    REG_BG3HOFS = 0; REG_BG3VOFS = 0;
    
    /* ---- Clear VRAM ---- */
    /* VRAM: 96KB from 0x06000000 to 0x06017FFF */
    volatile u32* vram = (volatile u32*)0x06000000;
    for (int i = 0; i < (96 * 1024) / 4; i++) {
        vram[i] = 0;
    }
    
    /* ---- Clear OAM (Object Attribute Memory) ---- */
    /* OAM: 1KB from 0x07000000 to 0x070003FF */
    volatile u32* oam = (volatile u32*)0x07000000;
    for (int i = 0; i < 128; i++) {
        oam[i] = 0;  /* All attributes 0 = sprite hidden (attribute 2 = 0, off-screen) */
    }
    
    /* ---- Clear Palette RAM ---- */
    /* Background palette: 512 bytes from 0x05000000 */
    volatile u16* pal = (volatile u16*)0x05000000;
    for (int i = 0; i < 512 / 2; i++) {
        pal[i] = 0;
    }
    /* Set color 0 to black, color 1 to white by default */
    PAL_BG[0] = 0x0000;  /* Black */
    PAL_BG[1] = 0x7FFF;  /* White (15-bit: 11111 11111 11111) */
    
    /* ---- Enable VBlank interrupt ---- */
    REG_DISPSTAT = DISPSTAT_VBL_IRQ;  /* Enable VBlank IRQ generation */
    REG_IE = IRQ_VBLANK;              /* Enable VBlank in interrupt controller */
    REG_IME = 1;                       /* Master interrupt enable */
    
    /* ---- Reset frame counter ---- */
    g_frame_count = 0;
}

/**
 * Wait for the current VBlank to start.
 * Spins until REG_DISPSTAT indicates VBlank active.
 */
void sys_wait_vblank(void) {
    /* Spin until VBlank flag is set */
    while (!(REG_DISPSTAT & DISPSTAT_VBL));
    /* Wait until VBlank ends (scanline returns to 0) */
    while (REG_DISPSTAT & DISPSTAT_VBL);
}

/**
 * Register a VBlank callback function.
 * Called automatically during each VBlank interrupt.
 */
void sys_set_vblank_callback(VBlankCallback cb) {
    s_vblank_cb = cb;
}

/**
 * Get the current frame count.
 */
u32 sys_get_frame_count(void) {
    return g_frame_count;
}

u32 sys_get_vblank_count(void) {
    return g_frame_count;
}

/**
 * Busy-wait for approximately 'cycles' CPU cycles.
 * Each loop iteration is ~3-5 cycles depending on pipeline.
 */
void sys_delay_cycles(u32 cycles) {
    volatile u32 count = cycles;
    while (count > 0) {
        count--;
    }
}

/**
 * Halt CPU until next interrupt fires.
 */
void sys_halt(void) {
    /* The ARM7TDMI halt instruction is not directly accessible in C. */
    /* We use a SWI call to BIOS HaltIntr or simply wait for VBlank. */
    sys_wait_vblank();
}

/**
 * Reset the game by jumping to the ROM entry point.
 */
void NORETURN sys_reset(void) {
    void (*reset)(void) = (void (*)(void))0x08000000;
    reset();
    /* Should never reach here */
    while (1);
}
