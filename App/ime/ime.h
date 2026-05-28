/* SPDX-License-Identifier: Apache-2.0
 * Pinyin IME state machine — input interface.
 *
 * Design principles (from architecture doc):
 *  - IME does NOT know how the UI draws; UI does NOT know how IME finds chars.
 *  - All IME state is in ImeCtx_t; callers own the context (no hidden globals).
 *  - Pinyin lookup: binary search on const MCU Flash table — zero SPI reads.
 *  - Font rendering (SPI Flash) is the caller's responsibility; IME only
 *    returns Unicode codepoints.
 *
 * Typical call sequence (channel-name editor):
 *
 *   ImeCtx_t ime;
 *   IME_Reset(&ime);
 *   // ... on key press:
 *   ImeState_t st = IME_Feed(&ime, key, false);
 *   // ... render based on st, ime.preedit, ime.candidates
 *   // ... when user selects a candidate (KEY_1..KEY_6 in CAND state):
 *   uint16_t cp = IME_PickCandidate(&ime, idx);  // Unicode codepoint
 *   // UTF-8 encode cp and append to edit buffer
 *
 * Input mode cycle (KEY_STAR):
 *   LOWER → UPPER → DIGIT → PINYIN → LOWER → …
 */

#ifndef IME_IME_H
#define IME_IME_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/keyboard.h"

/* -------------------------------------------------------------------------
 * IME states
 * --------------------------------------------------------------------- */
typedef enum {
    IME_IDLE      = 0,  /* no pending input; direct char / digit mode     */
    IME_LETTERS,        /* showing letter candidates for the last T9 key  */
    IME_PREEDIT,        /* preedit buffer growing; no full match yet      */
    IME_CANDIDATES,     /* pinyin matched; showing Unicode char candidates */
} ImeState_t;

/* Input mode (cycled by KEY_STAR) */
typedef enum {
    IME_MODE_LOWER  = 0,  /* a–z */
    IME_MODE_UPPER,       /* A–Z */
    IME_MODE_DIGIT,       /* 0–9 */
    IME_MODE_PINYIN,      /* Chinese pinyin */
    IME_MODE_COUNT
} ImeMode_t;

/* Maximum preedit syllable length */
#define IME_PREEDIT_MAX   8u
/* Candidates shown at once (1-indexed: KEY_1 … KEY_6) */
#define IME_CANDS_VISIBLE 6u

/* -------------------------------------------------------------------------
 * Context — one per active editor, caller-owned
 * --------------------------------------------------------------------- */
typedef struct {
    ImeMode_t  mode;

    /* --- LETTERS sub-state --- */
    KEY_Code_t letters_key;          /* KEY_2 … KEY_9 that was last pressed  */
    uint8_t    letters_count;        /* how many letters this key has         */
    uint8_t    letters_idx;          /* currently highlighted letter index    */
    char       letters_buf[5];       /* letters for current key, NUL-term     */

    /* --- PREEDIT / CANDIDATES sub-state --- */
    char       preedit[IME_PREEDIT_MAX + 1]; /* current syllable, NUL-term   */
    uint8_t    preedit_len;
    bool       no_match;             /* preedit typed but not in table        */

    uint16_t   candidates[IME_CANDS_VISIBLE]; /* Unicode codepoints           */
    uint8_t    cand_count;           /* valid entries in candidates[]          */
    uint8_t    cand_offset;          /* scroll offset (multiples of VISIBLE)  */
    uint8_t    cand_total;           /* total matches for current preedit     */
} ImeCtx_t;

/* -------------------------------------------------------------------------
 * API
 * --------------------------------------------------------------------- */

/* (Re)initialise context.  Call once before first use, or to clear state. */
void IME_Reset(ImeCtx_t *ctx);

/* Cycle input mode: LOWER → UPPER → DIGIT → PINYIN → LOWER.
 * Returns new mode. */
ImeMode_t IME_CycleMode(ImeCtx_t *ctx);

/* Feed one key press to the IME.
 * `held` = true when the key was held (long-press).
 * Returns the new IME state.
 *
 * Caller should then read ctx fields to update the display:
 *   IME_LETTERS    → ctx->letters_buf, ctx->letters_idx
 *   IME_PREEDIT    → ctx->preedit, ctx->no_match
 *   IME_CANDIDATES → ctx->preedit, ctx->candidates, ctx->cand_count,
 *                    ctx->cand_offset, ctx->cand_total
 *
 * KEY_STAR is consumed here (cycles mode; does not emit a char).
 * KEY_EXIT discards preedit / letters without changing mode.
 * All other keys are handled inside; returns IME_IDLE for "pass-through"
 * keys that the caller should process normally (e.g. save, cancel).
 *
 * Return value of IME_IDLE with key not being KEY_STAR/KEY_EXIT means
 * the key was NOT consumed — caller must handle it (e.g. move cursor). */
ImeState_t IME_Feed(ImeCtx_t *ctx, KEY_Code_t key, bool held);

/* In IME_CANDIDATES: pick the candidate at visible index idx (0-based).
 * Returns the Unicode codepoint (U+4Exx etc.) or 0 if idx is out of range.
 * Also resets the preedit / candidate state (sets state back to IDLE). */
uint16_t IME_PickCandidate(ImeCtx_t *ctx, uint8_t idx);

/* In IME_CANDIDATES: scroll forward / back one page of IME_CANDS_VISIBLE.
 * Returns new cand_offset. */
uint8_t IME_ScrollCandidates(ImeCtx_t *ctx, int8_t direction);

/* Backspace one unit:
 *   IME_CANDIDATES / IME_PREEDIT → remove last preedit char (→ re-search or IDLE)
 *   IME_LETTERS                  → cancel letter selection (→ IDLE)
 *   IME_IDLE                     → returns false (caller should delete from edit buf)
 * Returns true if IME consumed the backspace. */
bool IME_Backspace(ImeCtx_t *ctx);

/* Returns a short mode indicator string suitable for the status bar:
 *   "拼" / "abc" / "ABC" / "123" */
const char *IME_ModeLabel(const ImeCtx_t *ctx);

#endif /* IME_IME_H */
