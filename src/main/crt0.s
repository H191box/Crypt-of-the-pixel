@ =============================================================================
@ crt0.s — GBA Startup Code with embedded ROM header
@ 
@ Entry point: _start (ARM mode, placed at 0x08000000)
@ The first 200 bytes are the GBA ROM header with Nintendo logo.
@ After the header, real_start sets up the system and calls main().
@ =============================================================================

    .section .text
    .global _start
    .arm

_start:
    @ ==== GBA ROM Header (200 bytes = 50 words) ====
    @ Word 0: ARM branch to real_start (skip past header)
    b       real_start

    @ Words 1-39: Nintendo logo (156 bytes, required by BIOS)
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

    @ Word 40: Game title "CRYPTPIXEL" (12 bytes) + "CTPX" (4 bytes) 
    @ Packed as 4 bytes at a time: "RYPT" "PIXE" "LCTP" "X\0\0\0"
    .ascii "RYPT"
    .ascii "PIXE"
    .ascii "LCTP"
    .ascii "X\0\0\0"

    @ Word 44: Maker code "00" (2 bytes) + fixed 0x96 + main unit 0x00 + device 0x00
    .byte 0x30, 0x30              @ Maker code "00"
    .byte 0x96                    @ Fixed value
    .byte 0x00                    @ Main unit code
    .byte 0x00                    @ Device type
    .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  @ Reserved (7 bytes)

    @ Word 48: Version (1 byte) + Complement (1 byte) + Checksum (2 bytes)
    .byte 0x00                    @ Version
    .byte 0xB7                    @ Complement check (sum of 0xA0-0xBC must be 0)
    .hword 0x0000                 @ Checksum (optional, 0 = skip)

    @ ==== Real startup code begins here (offset ~200 bytes from ROM start) ====
    .align 2
real_start:
    @ ---- Step 1: Set stack pointer ----
    @ IWRAM: 0x03000000 - 0x03007FFF. SP at top, leaving room for BIOS.
    ldr     sp, =0x03007F00

    @ ---- Step 2: Copy .data from ROM to EWRAM ----
    ldr     r0, =__data_load
    ldr     r1, =__data_start
    ldr     r2, =__data_end
1:
    cmp     r1, r2
    bge     2f
    ldr     r3, [r0], #4
    str     r3, [r1], #4
    b       1b
2:

    @ ---- Step 3: Clear .bss ----
    ldr     r0, =__bss_start
    ldr     r1, =__bss_end
    mov     r2, #0
3:
    cmp     r0, r1
    bge     4f
    str     r2, [r0], #4
    b       3b
4:

    @ ---- Step 4: Branch to main() (Thumb mode) ----
    ldr     r0, =main
    bx      r0

    @ ---- Hang if main returns ----
hang:
    b       hang

    .ltorg
