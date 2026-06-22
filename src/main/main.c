/**
 * main.c — Game Boy Advance Entry Point
 */

#include "../engine/gba_types.h"
#include "../engine/gba_header.h"
#include "../engine/system.h"
#include "../engine/input.h"
#include "../game/game_state.h"

int main(void) {
    /* === DIAGNOSTIC: Fill screen with RED in Mode 3 === */
    /* Mode 3: 240x160 pixels, 16-bit color, direct to VRAM */
    /* This proves main() is actually executing */
    REG_DISPCNT = 0x0003;  /* Mode 3 + BG2 enable */
    
    /* Write red pixels to entire VRAM (240x160 = 38400 halfwords) */
    /* In Mode 3, each pixel is one 16-bit value: 0x1F (red max) */
    /* Use 32-bit writes for speed: two pixels at once */
    volatile u32* vram = (volatile u32*)0x06000000;
    u32 red = 0x001F001F;  /* Two red pixels packed */
    int i;
    for (i = 0; i < 38400 / 2; i++) {
        vram[i] = red;
    }
    
    /* Wait a few seconds so we can see the red screen */
    volatile u32 count = 0;
    for (count = 0; count < 0x400000; count++) {
        /* busy wait ~3 seconds */
    }
    
    /* === Now try the real game === */
    game_init();
    
    while (1) {
        input_update();
        game_update();
        game_render();
        sys_wait_vblank();
    }
    
    return 0;
}
