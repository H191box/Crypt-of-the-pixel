@ =============================================================================
@ crt0.s — GBA Startup + ROM Header (pure ARM diagnostic)
@ 
@ BUG FIX 1: Game title was "RYPTPIXELCTP" instead of "CRYPTPIXEL"
@ BUG FIX 2: Complement check (0xBD) must make sum(0xA0..0xBC) = 0 mod 256
@ BUG FIX 3: Mode 3 requires BG2 enable (bit 10) → REG_DISPCNT = 0x0403
@ =============================================================================

    .section .text
    .global _start
    .arm

_start:
    @ ==== GBA ROM Header (192 bytes at offset 0x00-0xBF) ====
    b       real_start

    @ Nintendo logo (156 bytes = 39 words, offset 0x04-0x9F)
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

    @ Offset 0xA0: Game title "CRYPTPIXEL" (12 bytes)
    .ascii "CRYP"
    .ascii "TPIX"
    .ascii "EL\0\0"

    @ Offset 0xAC: Game code "CTPX" (4 bytes)
    .ascii "CTPX"

    @ Offset 0xB0: Maker code "00"
    .byte 0x30, 0x30

    @ Offset 0xB2: Fixed value 0x96
    .byte 0x96

    @ Offset 0xB3: Main unit code
    .byte 0x00

    @ Offset 0xB4: Device type
    .byte 0x00

    @ Offset 0xB5-0xBB: Reserved (7 bytes)
    .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    @ Offset 0xBC: Version
    .byte 0x00

    @ Offset 0xBD: Complement check
    @ Sum of bytes 0xA0-0xBC = 0x49. 0x49 + 0xB7 = 0x100 ≡ 0 mod 256.
    .byte 0xB7

    @ Offset 0xBE-0xBF: Checksum
    .hword 0x0000

    @ ==== Real code (offset 0xC0) ====
    .align 2
real_start:
    @ -- Enable Mode 3 + BG2 --
    @ REG_DISPCNT = 0x0403 (Mode 3 = bits[2:0], BG2 = bit 10)
    mov     r0, #0x04
    mov     r0, r0, lsl #26        @ r0 = 0x04000000
    mov     r1, #3                 @ Mode 3
    mov     r2, #1
    orr     r1, r1, r2, lsl #10    @ r1 = 3 | 0x0400 = 0x0403
    str     r1, [r0, #0]

    @ -- Fill VRAM with RED (Mode 3: 240x160, 16-bit pixels) --
    @ 19200 32-bit writes cover entire framebuffer (38400 * 2 bytes)
    @ Outer=150, Inner=128: 150*128 = 19200
    mov     r3, #0x06
    mov     r3, r3, lsl #26        @ r3 = 0x06000000 (VRAM)
    mov     r4, #0x1F              @ Red
    orr     r4, r4, r4, lsl #16    @ r4 = 0x001F001F
    mov     r6, #0                 @ byte offset

    mov     r7, #150               @ outer count

outer_loop:
    mov     r5, #128               @ inner count
inner_loop:
    str     r4, [r3, r6]
    add     r6, r6, #4
    subs    r5, r5, #1
    bne     inner_loop
    subs    r7, r7, #1
    bne     outer_loop

    @ -- Hang --
hang:
    b       hang
