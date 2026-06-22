OUTPUT_ARCH(arm)
ENTRY(_start)
MEMORY {
  rom   : ORIGIN = 0x08000000, LENGTH = 32M
  ewram : ORIGIN = 0x02000000, LENGTH = 256K
  iwram : ORIGIN = 0x03000000, LENGTH = 32K
}
SECTIONS {
  .rom  0x08000000 : { *(.header) *(.text) *(.rodata) } > rom
  .data : {
    __data_start = . ;
    *(.data)
    __data_end = . ;
  } > ewram AT> rom
  __data_load = LOADADDR(.data) ;
  .bss  : {
    __bss_start = . ;
    *(.bss) *(COMMON)
    __bss_end = . ;
  } > ewram
  .iwram : { *(.iwram) *(.iwram.*) } > iwram
  /DISCARD/ : { *(.ARM.attributes) }
}
