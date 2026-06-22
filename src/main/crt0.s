@ =============================================================================
@ crt0.s — GBA Startup Code (ARM mode entry point)
@ 
@ The GBA BIOS loads the ROM header at 0x08000000 and jumps to the
@ entry point stored in the first 4 bytes. This file provides _start
@ which is the very first code that executes.
@ 
@ Responsibilities:
@   1. Set up the stack pointer (SP) to top of IWRAM
@   2. Copy .data section from ROM to EWRAM
@   3. Clear .bss section in EWRAM
@   4. Branch to main() in Thumb mode
@ =============================================================================

    .section .text
    .global _start
    .arm                    @ Entry MUST be in ARM mode

_start:
    @ ---- Set up stack pointer ----
    @ IWRAM: 0x03000000 - 0x03007FFF (32KB)
    @ Stack grows downward, so SP = 0x03007F00 (leaves 256 bytes for BIOS)
    ldr     sp, =0x03007F00

    @ ---- Copy .data section from ROM to EWRAM ----
    @ The linker places .data in EWRAM but loads it from ROM.
    @ linker symbols: __data_load (ROM addr), __data_start (EWRAM addr), __data_end
    ldr     r0, =__data_load      @ Source: ROM address of .data
    ldr     r1, =__data_start     @ Dest: EWRAM address of .data
    ldr     r2, =__data_end       @ End of .data in EWRAM
copy_data:
    cmp     r1, r2
    bge     data_done
    ldr     r3, [r0], #4
    str     r3, [r1], #4
    b       copy_data
data_done:

    @ ---- Clear .bss section ----
    ldr     r0, =__bss_start
    ldr     r1, =__bss_end
    mov     r2, #0
clear_bss:
    cmp     r0, r1
    bge     bss_done
    str     r2, [r0], #4
    b       clear_bss
bss_done:

    @ ---- Branch to main() in Thumb mode ----
    @ BX switches between ARM and Thumb based on bit 0 of the address
    ldr     r0, =main
    bx      r0

    @ ---- Should never reach here ----
hang:
    b       hang
