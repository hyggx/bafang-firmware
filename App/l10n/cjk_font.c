/* SPDX-License-Identifier: Apache-2.0
 * CJK 12×12 glyph cache with binary-search SPI Flash lookup.
 *
 * Design principles (from Dondji performance audit):
 *  - NEVER do a linear SPI scan for a glyph lookup.
 *    Binary search: O(log₂ 6000) ≈ 13 SPI reads instead of 6000.
 *  - All SPI reads are exactly CJK_INDEX_ENTRY_SZ (4 bytes) or
 *    CJK_GLYPH_BYTES (24 bytes), both < 16 bytes threshold for DMA.
 *    TODO: batch to ≥ 16 bytes if SPI DMA perf matters in future.
 *  - 16-slot LRU cache in RAM (416 bytes) eliminates repeated SPI reads
 *    for channel names displayed every frame (~25 Hz).
 *  - The render path MUST NOT call CJK_GetGlyph from an ISR or with
 *    SysTick disabled; SPI polling at 24 MHz is synchronous but safe in
 *    the main loop.
 */

#include "cjk_font.h"
#include <string.h>

extern const uint8_t g_cjk_font_data[];

static CjkCache_t s_cache;

/* -------------------------------------------------------------------------
 * Internal helpers
 * ------------------------------------------------------------------------- */

/* Binary search in the sorted index table stored in SPI Flash.
 * Returns the 0-based glyph index, or UINT16_MAX if not found.             */
static uint16_t s_bsearch(uint16_t cp)
{
    if (s_cache.glyph_count == 0)
        return UINT16_MAX;

    uint16_t lo = 0;
    uint16_t hi = s_cache.glyph_count - 1u;

    while (lo <= hi) {
        uint16_t mid = (uint16_t)(lo + ((hi - lo) >> 1));

        CjkIndexEntry_t entry;
        uint32_t addr = CJK_HEADER_SZ
                        + (uint32_t)mid * CJK_INDEX_ENTRY_SZ;
        memcpy(&entry, g_cjk_font_data + addr, sizeof(entry));

        if (entry.codepoint == cp)
            return mid;
        else if (entry.codepoint < cp)
            lo = mid + 1u;
        else {
            if (mid == 0) break;
            hi = mid - 1u;
        }
    }
    return UINT16_MAX;
}

/* Promote slot `slot_idx` to MRU position in lru_order[].                  */
static void s_lru_promote(uint8_t slot_idx)
{
    /* Find current position */
    uint8_t pos = 0;
    for (uint8_t i = 0; i < CJK_CACHE_SLOTS; i++) {
        if (s_cache.lru_order[i] == slot_idx) {
            pos = i;
            break;
        }
    }
    /* Shift everything left to close the gap, then put at front */
    for (uint8_t i = pos; i > 0; i--)
        s_cache.lru_order[i] = s_cache.lru_order[i - 1u];
    s_cache.lru_order[0] = slot_idx;
}

/* Return the LRU (oldest) slot index.                                       */
static uint8_t s_lru_victim(void)
{
    return s_cache.lru_order[CJK_CACHE_SLOTS - 1u];
}

/* -------------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------------- */

bool CJK_Init(void)
{
    memset(&s_cache, 0, sizeof(s_cache));

    /* Initialise LRU order: slot 0 = MRU, slot N-1 = LRU                   */
    for (uint8_t i = 0; i < CJK_CACHE_SLOTS; i++)
        s_cache.lru_order[i] = i;

    /* Read and validate font header from embedded array */
    CjkFontHeader_t hdr;
    memcpy(&hdr, g_cjk_font_data, sizeof(hdr));

    if (hdr.magic   != CJK_FONT_MAGIC   ||
        hdr.version != CJK_FONT_VERSION  ||
        hdr.glyph_count == 0)
        return false;

    s_cache.glyph_count  = hdr.glyph_count;
    s_cache.bitmap_off   = hdr.bitmap_off;
    s_cache.header_valid = true;
    return true;
}

bool CJK_FontAvailable(void)
{
    return s_cache.header_valid;
}

bool CJK_GetGlyph(uint16_t cp, uint8_t out_bitmap[CJK_GLYPH_BYTES])
{
    if (!s_cache.header_valid)
        return false;

    /* 1. Cache hit? */
    for (uint8_t i = 0; i < CJK_CACHE_SLOTS; i++) {
        if (s_cache.codepoints[i] == cp && cp != 0) {
            memcpy(out_bitmap, s_cache.bitmaps[i], CJK_GLYPH_BYTES);
            s_lru_promote(i);
            return true;
        }
    }

    /* 2. Binary search in SPI Flash index */
    uint16_t glyph_idx = s_bsearch(cp);
    if (glyph_idx == UINT16_MAX)
        return false;   /* glyph not in font */

    /* 3. Read bitmap from embedded array */
    uint32_t bmp_addr = s_cache.bitmap_off
                        + (uint32_t)glyph_idx * CJK_GLYPH_BYTES;
    uint8_t bitmap[CJK_GLYPH_BYTES];
    memcpy(bitmap, g_cjk_font_data + bmp_addr, CJK_GLYPH_BYTES);

    /* 4. Evict LRU slot and store new glyph */
    uint8_t victim = s_lru_victim();
    s_cache.codepoints[victim] = cp;
    memcpy(s_cache.bitmaps[victim], bitmap, CJK_GLYPH_BYTES);
    s_lru_promote(victim);

    memcpy(out_bitmap, bitmap, CJK_GLYPH_BYTES);
    return true;
}

void CJK_Invalidate(void)
{
    memset(&s_cache, 0, sizeof(s_cache));
    for (uint8_t i = 0; i < CJK_CACHE_SLOTS; i++)
        s_cache.lru_order[i] = i;
}
