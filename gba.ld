OUTPUT_ARCH(arm)
ENTRY(_start)
MEMORY {
  rom   : ORIGIN = 0x08000000, LENGTH = 32M
  ewram : ORIGIN = 0x02000000, LENGTH = 256K
  iwram : ORIGIN = 0x03000000, LENGTH = 32K
}
SECTIONS {
  /* .text must be first so _start (with ROM header) is at 0x08000000 */
  .text 0x08000000 : {
    *(.text)
    *(.rodata)
  } > rom

  /* .data: initialized globals, stored in ROM, copied to EWRAM by crt0 */
  .data : {
    __data_start = . ;
    *(.data)
    __data_end = . ;
  } > ewram AT> rom
  __data_load = LOADADDR(.data) ;

  /* .bss: uninitialized globals, NOLOAD so objcopy doesn't include them */
  .bss (NOLOAD) : {
    __bss_start = . ;
    *(.bss)
    *(COMMON)
    __bss_end = . ;
  } > ewram

  /* .iwram: code/data placed in IWRAM for speed, NOLOAD */
  .iwram (NOLOAD) : {
    *(.iwram)
    *(.iwram.*)
  } > iwram

  /* Discard unwanted sections */
  /DISCARD/ : {
    *(.ARM.attributes)
    *(.comment)
    *(.note*)
  }
}
