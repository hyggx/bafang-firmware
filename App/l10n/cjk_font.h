#ifndef L10N_CJK_FONT_H
#define L10N_CJK_FONT_H

#include <stdint.h>
#include <stdbool.h>

/* ==========================================================================
 * CJK 12×12 glyph database — SPI Flash layout
 *
 * Base address: 0x010000 (128 KB reserved, 0x010000–0x02FFFF)
 *
 * Layout:
 *
 *   [0x000]  Header       (16 bytes)
 *   [0x010]  Index table  (glyph_count × 4 bytes, sorted by codepoint ASC)
 *   [0x010 + glyph_count*4]  Bitmap data (glyph_count × CJK_GLYPH_BYTES)
 *
 * Header (16 bytes):
 *   u16  magic       0x464D ("FM")
 *   u8   version     1
 *   u8   _reserved   0
 *   u16  glyph_count number of glyphs in this database
 *   u16  _reserved2  0
 *   u32  bitmap_off  byte offset from base to first bitmap (= 0x10 + glyph_count*4)
 *   u32  crc32       CRC-32 of (header[0..11] ++ index ++ bitmaps)
 *
 * Index entry (4 bytes, sorted ascending by .codepoint):
 *   u16  codepoint   Unicode codepoint (U+4E00..U+9FFF for CJK Unified)
 *   u16  _reserved   0  (reserved for future glyph flags / variant index)
 *
 * Bitmap (CJK_GLYPH_BYTES = 24 bytes per glyph):
 *   12 rows × 2 bytes/row, MSB of byte 0 is pixel (col=0, row=N).
 *   Bit order: bit 15 = col 0, bit 4 = col 11 (right-padded with zeros).
 *
 * Binary search: given target codepoint T, find index i such that
 *   index[i].codepoint == T  →  bitmap address = base + bitmap_off + i*24
 * Total reads for one lookup: O(log₂ N) × 4 bytes = ≤ 14 reads for 6000 glyphs.
 * ========================================================================== */

/* Font is embedded in MCU internal flash as g_cjk_font_data[] (cjk_font_data.c).
 * All offsets below are relative to the start of that array.               */
extern const uint8_t g_cjk_font_data[];
#define CJK_FONT_MAGIC      0x464Du     /* "FM" little-endian                 */
#define CJK_FONT_VERSION    1u
#define CJK_GLYPH_W         12u         /* pixels per row                     */
#define CJK_GLYPH_H         12u         /* rows per glyph                     */
#define CJK_GLYPH_BYTES     24u         /* 12 rows × 2 bytes                  */
#define CJK_INDEX_ENTRY_SZ  4u          /* bytes per index entry              */
#define CJK_HEADER_SZ       16u         /* bytes                              */

typedef struct __attribute__((packed)) {
    uint16_t magic;
    uint8_t  version;
    uint8_t  _reserved;
    uint16_t glyph_count;
    uint16_t _reserved2;
    uint32_t bitmap_off;    /* offset from array start to bitmap[0]          */
    uint32_t crc32;
} CjkFontHeader_t;

typedef struct __attribute__((packed)) {
    uint16_t codepoint;
    uint16_t _reserved;
} CjkIndexEntry_t;

/* ==========================================================================
 * LRU glyph cache
 *
 * 16 slots, each holds one 12×12 glyph bitmap (24 bytes) + its codepoint.
 * Total RAM: 16 × (2 + 24) = 416 bytes.
 *
 * For channel-name display, the same 3–5 glyphs are requested every frame.
 * A 16-slot LRU achieves near-100% hit rate for typical channel names.
 * ========================================================================== */
#define CJK_CACHE_SLOTS     16u

/* Opaque state — do not access directly; use CJK_* functions below. */
typedef struct {
    uint16_t codepoints[CJK_CACHE_SLOTS];  /* 0x0000 = empty slot            */
    uint8_t  bitmaps[CJK_CACHE_SLOTS][CJK_GLYPH_BYTES];
    uint8_t  lru_order[CJK_CACHE_SLOTS];   /* lru_order[0] = MRU slot index  */
    uint16_t glyph_count;                  /* from font header, 0 = no font  */
    uint32_t bitmap_off;                   /* cached from header             */
    bool     header_valid;
} CjkCache_t;

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

/* Initialise the cache and verify the font header in SPI Flash.
 * Returns true if a valid font database is present.
 * Must be called once after SPI Flash is available (after board init).
 * Font must be pre-written with: python tools/serialtool/flash_font.py    */
bool CJK_Init(void);

/* Returns true if a valid font is present in SPI Flash.                     */
bool CJK_FontAvailable(void);

/* Look up glyph for Unicode codepoint `cp`.
 * On hit (cache or SPI Flash): copies CJK_GLYPH_BYTES into `out_bitmap`
 * and returns true.
 * On miss (glyph not in font): returns false; `out_bitmap` is unchanged.    */
bool CJK_GetGlyph(uint16_t cp, uint8_t out_bitmap[CJK_GLYPH_BYTES]);

/* Invalidate the cache (call after writing a new font to SPI Flash).        */
void CJK_Invalidate(void);

#endif /* L10N_CJK_FONT_H */
