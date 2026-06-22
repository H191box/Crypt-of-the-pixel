@ =============================================================================
@ crt0.s — GBA Startup + ROM Header (pure ARM diagnostic)
@ 
@ FIX: Use literal pool loads (ldr =val) instead of mov+lsl arithmetic
@      to guarantee correct hardware register addresses.
@
@ Previous bug: mov r0, #4; lsl #26 → 0x10000000 (WRONG, not 0x04000000)
@              mov r3, #6; lsl #26 → 0x18000000 (WRONG, not 0x06000000)
@ =============================================================================

    .section .text
    .global _start
    .arm

_start:
    @ ==== GBA ROM Header (192 bytes) ====
    b       real_start

    @ Nintendo logo (156 bytes)
    .word 0x24FFAE51, 0x699AA221, 0x3D84820A, 0x84E409AD
    .word 0x11248B98, 0xC0817F21, 0xA352BE19, 0x9309CE20
    .word 0x10464A4A, 0xF82731EC, 0x58C7E833, 0x82E3CEBF
    .word 0x85F4DF94, 0xCE4B09C1, 0x94568AC0, 0x1372A7FC
    .word 0x9F844D73, 0xA3CA9A61, 0x5897A327, 0xFC039876
    .word 0x231DC761, 0x0304AE56, 0xBF388400, 0x40A70EFD
    .word 0xFF52FE03, 0x6F9530F1, 0x97FBC085, 0x60D68025
    .word 0xA963BE03, 0x014E38E2, 0xF9A234FF, 0xBB3E0344
    .word 0x780090CB, 0x88113A94, 0x65C07C63, 0x87F03CAF
    .word 0xD625E48B, 0x380AAC72, 0x21D4F807

    @ Offset 0xA0: Game title "CRYPTPIXEL"
    .ascii "CRYP"
    .ascii "TPIX"
    .ascii "EL\0\0"

    @ Offset 0xAC: Game code
    .ascii "CTPX"

    @ Offset 0xB0: Maker code
    .byte 0x30, 0x30

    @ Offset 0xB2: Fixed 0x96
    .byte 0x96

    @ Offset 0xB3-0xB4: Main unit + device
    .byte 0x00, 0x00

    @ Offset 0xB5-0xBB: Reserved
    .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    @ Offset 0xBC: Version
    .byte 0x00

    @ Offset 0xBD: Complement (sum 0xA0-0xBC + this = 0 mod 256)
    .byte 0xB7

    @ Offset 0xBE-0xBF: Checksum
    .hword 0x0000

    @ ==== Code at offset 0xC0 ====
    .align 2
real_start:
    @ -- Set stack pointer --
    ldr     sp, =0x03007F00

    @ -- DIAGNOSTIC: Fill screen RED (Mode 3, pure ARM, no Thumb) --
    @ Enable Mode 3 + BG2: REG_DISPCNT = 0x0403
    ldr     r0, =0x04000000
    ldr     r1, =0x0403
    str     r1, [r0, #0]

    @ Fill VRAM with red pixels
    @ Mode 3: 240x160, 16-bit per pixel, VRAM at 0x06000000
    @ Two pixels per 32-bit store: 0x001F001F
    @ 19200 stores cover entire screen
    ldr     r3, =0x06000000     @ VRAM base
    ldr     r4, =0x001F001F     @ Two red pixels
    ldr     r2, =0             @ Offset counter

    ldr     r7, =150            @ Outer loop count

fill_outer:
    ldr     r5, =128            @ Inner loop count
fill_inner:
    str     r4, [r3, r2]
    add     r2, r2, #4
    subs    r5, r5, #1
    bne     fill_inner
    subs    r7, r7, #1
    bne     fill_outer

    @ -- Hang forever --
hang:
    b       hang

    .ltorg
