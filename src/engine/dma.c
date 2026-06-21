/**
 * dma.c — DMA Transfer Implementation
 * 
 * Uses DMA channel 3 for most operations since it has no
 * timing restrictions (DMA0/1 can only fire during specific
 * display periods). DMA3 is fastest and most flexible.
 */

#include "gba_header.h"
#include "gba_types.h"
#include "dma.h"

/**
 * Internal: Set up and fire a DMA transfer on the given channel.
 */
static void dma_transfer(u8 channel, const void* src, void* dst, 
                           u32 count, u32 ctrl_flags) {
    volatile u32* sad = (volatile u32*)(REG_BASE + 0xB0 + channel * 12);
    volatile u32* dad = (volatile u32*)(REG_BASE + 0xB4 + channel * 12);
    volatile u32* cnt = (volatile u32*)(REG_BASE + 0xB8 + channel * 12);
    
    /* DMA must be disabled before configuration */
    *cnt = 0;
    
    /* Set source and destination */
    *sad = (u32)src;
    *dad = (u32)dst;
    
    /* Set word count (must be at least 1, and max 0x10000 for 16/32-bit) */
    if (count == 0) count = 1;
    if (count > 0x10000) count = 0x10000;
    
    /* Write control register: count in lower 16 bits, flags in upper */
    *cnt = (count - 1) | ctrl_flags | DMA_ENABLE;
    
    /* Wait for DMA to complete */
    while (*cnt & DMA_ENABLE);
}

/**
 * 32-bit DMA copy (fastest, requires 4-byte alignment).
 */
void dma_copy_32(void* dst, const void* src, u32 count) {
    if (count == 0) return;
    dma_transfer(DMA_CHANNEL_3, src, dst, count,
                 DMA_32 | DMA_SRC_INC | DMA_DST_INC);
}

/**
 * 16-bit DMA copy.
 */
void dma_copy_16(void* dst, const void* src, u32 count) {
    if (count == 0) return;
    dma_transfer(DMA_CHANNEL_3, src, dst, count,
                 DMA_16 | DMA_SRC_INC | DMA_DST_INC);
}

/**
 * Fill memory with a 32-bit value via DMA (source is fixed).
 */
void dma_memset_32(void* dst, u32 val, u32 count) {
    if (count == 0) return;
    dma_transfer(DMA_CHANNEL_3, &val, dst, count,
                 DMA_32 | DMA_SRC_FIXED | DMA_DST_INC);
}

/**
 * Fill memory with a 16-bit value via DMA.
 */
void dma_memset_16(void* dst, u16 val, u32 count) {
    if (count == 0) return;
    /* We need a 32-bit aligned temp for the source. Use a static variable. */
    static u32 temp_val;
    temp_val = val | (val << 16);  /* Duplicate to both half-words */
    /* Actually, for 16-bit DMA fill, we need the source to hold val.
     * DMA in 16-bit mode reads 16 bits from source. */
    /* Let's just use the stack variable directly */
    dma_transfer(DMA_CHANNEL_3, &val, dst, count,
                 DMA_16 | DMA_SRC_FIXED | DMA_DST_INC);
}

/**
 * Generic DMA memcpy — picks 16 or 32-bit based on alignment.
 */
void dma_memcpy(void* dst, const void* src, u32 size) {
    if (size == 0) return;
    
    /* Check if both addresses are word-aligned */
    if (((u32)dst & 3) == 0 && ((u32)src & 3) == 0 && (size & 3) == 0) {
        dma_copy_32(dst, src, size / 4);
    } else {
        dma_copy_16(dst, src, size / 2);
    }
}

/**
 * Copy data to VRAM. Uses DMA3 (no timing restriction).
 */
void dma_to_vram(void* dst, const void* src, u32 size) {
    dma_memcpy(dst, src, size);
}

/**
 * Copy 1024 bytes of OAM data to OAM.
 */
void dma_to_oam(const void* oam_data) {
    dma_copy_32((void*)0x07000000, oam_data, 1024 / 4);
}

/**
 * Copy palette data to BG or OBJ palette.
 */
void dma_to_palette(const u16* pal_data, u16 count, bool bg_pal) {
    void* dst = bg_pal ? (void*)PAL_BG : (void*)PAL_OBJ;
    dma_copy_16(dst, pal_data, count);
}
