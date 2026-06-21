/**
 * dma.h — DMA Transfer Utilities for GBA
 * 
 * The GBA has 4 DMA channels (DMA0-DMA3) that can transfer data
 * between memory regions without CPU intervention. DMA3 is the
 * fastest and most versatile. DMA0/DMA1 can only run during
 * VBlank/HBlank.
 */

#ifndef DMA_H
#define DMA_H

#include "gba_types.h"

/* DMA channel selection */
#define DMA_CHANNEL_0   0
#define DMA_CHANNEL_1   1
#define DMA_CHANNEL_2   2
#define DMA_CHANNEL_3   3

/* ---- Functions ---- */

/**
 * Copy data using DMA (32-bit transfers, fastest).
 * Only works for word-aligned addresses and word-multiple sizes.
 * @param dst   Destination address (must be word-aligned)
 * @param src   Source address (must be word-aligned)
 * @param count Number of 32-bit words to copy
 */
void dma_copy_32(void* dst, const void* src, u32 count);

/**
 * Copy data using DMA (16-bit transfers).
 * Works for half-word-aligned addresses.
 * @param dst   Destination address (must be half-word aligned)
 * @param src   Source address (must be half-word aligned)
 * @param count Number of 16-bit half-words to copy
 */
void dma_copy_16(void* dst, const void* src, u32 count);

/**
 * Fill memory with a repeating 32-bit pattern using DMA.
 * @param dst   Destination address
 * @param val   32-bit fill value
 * @param count Number of 32-bit words to fill
 */
void dma_memset_32(void* dst, u32 val, u32 count);

/**
 * Fill memory with a repeating 16-bit pattern using DMA.
 * @param dst   Destination address
 * @param val   16-bit fill value
 * @param count Number of 16-bit half-words to fill
 */
void dma_memset_16(void* dst, u16 val, u32 count);

/**
 * Generic DMA memcpy (auto-selects 16 or 32-bit based on alignment).
 * @param dst   Destination
 * @param src   Source
 * @param size  Size in bytes
 */
void dma_memcpy(void* dst, const void* src, u32 size);

/**
 * Fast copy to VRAM using DMA channel 3.
 * Safe for use outside of VBlank (DMA3 has no timing restrictions).
 * @param dst   VRAM destination address
 * @param src   Source address
 * @param size  Size in bytes
 */
void dma_to_vram(void* dst, const void* src, u32 size);

/**
 * Fast copy to OAM using DMA channel 3.
 * @param oam_data  Source buffer (1024 bytes of OAM data)
 */
void dma_to_oam(const void* oam_data);

/**
 * Copy palette data using DMA channel 3.
 * @param pal_data  Source palette data
 * @param count     Number of 16-bit colors
 * @param bg_pal    TRUE for background palette, FALSE for sprite palette
 */
void dma_to_palette(const u16* pal_data, u16 count, bool bg_pal);

#endif /* DMA_H */
