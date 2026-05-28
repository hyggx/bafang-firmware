/* SPDX-License-Identifier: Apache-2.0
 * Pinyin IME state machine implementation.
 *
 * Key design decisions vs Dondji:
 *  - Binary search on const pinyin table in MCU Flash (O(log 334) ≈ 9 steps).
 *    Dondji scans 334 entries linearly via ~1000 SPI reads per keystroke.
 *  - All state in caller-owned ImeCtx_t; no file-scope globals.
 *  - No UI code here; IME only manipulates the context and returns state.
 *  - T9 disambiguation: KEY_2-9 shows letters, KEY_1-4 selects; then
 *    auto-search triggers after each letter addition.
 */

#include <string.h>
#include "ime/ime.h"
#include "ime/pinyin_table.h"

/* -------------------------------------------------------------------------
 * PY_BSearch — binary search in the sorted pinyin table
 * --------------------------------------------------------------------- */
const PinyinEntry_t *PY_BSearch(const char *syllable)
{
    int lo = 0;
    int hi = (int)kPyTableLen - 1;
    while (lo <= hi) {
        int mid = lo + ((hi - lo) >> 1);
        int cmp = __builtin_strcmp(kPyTable[mid].syllable, syllable);
        if (cmp == 0) return &kPyTable[mid];
        if (cmp < 0)  lo = mid + 1;
        else          hi = mid - 1;
    }
    return NULL;
}

/* -------------------------------------------------------------------------
 * Internal helpers
 * --------------------------------------------------------------------- */

/* Letters available on each T9 key (KEY_2 … KEY_9).
 * Index 0 → KEY_2 ("abc"), …, index 7 → KEY_9 ("wxyz"). */
static const char * const kT9Letters[8] = {
    "abc", "def", "ghi", "jkl", "mno", "pqrs", "tuv", "wxyz"
};

static void s_set_letters(ImeCtx_t *ctx, KEY_Code_t key)
{
    if (key < KEY_2 || key > KEY_9) return;
    const char *letters = kT9Letters[key - KEY_2];
    uint8_t n = 0;
    while (letters[n] && n < (uint8_t)(sizeof(ctx->letters_buf) - 1u)) {
        ctx->letters_buf[n] = letters[n];
        n++;
    }
    ctx->letters_buf[n]  = '\0';
    ctx->letters_count   = n;
    ctx->letters_key     = key;
    ctx->letters_idx     = 0;
}

/* Add a letter to the preedit buffer, then search. */
static ImeState_t s_add_to_preedit(ImeCtx_t *ctx, char letter)
{
    if (ctx->preedit_len >= IME_PREEDIT_MAX)
        return IME_PREEDIT;

    ctx->preedit[ctx->preedit_len++] = letter;
    ctx->preedit[ctx->preedit_len]   = '\0';
    ctx->letters_count               = 0;
    ctx->no_match                    = false;

    const PinyinEntry_t *entry = PY_BSearch(ctx->preedit);
    if (entry) {
        /* Exact syllable match — populate candidates */
        ctx->cand_offset = 0;
        ctx->cand_total  = entry->count;
        uint8_t vis = (entry->count < IME_CANDS_VISIBLE)
                      ? entry->count : IME_CANDS_VISIBLE;
        for (uint8_t i = 0; i < vis; i++)
            ctx->candidates[i] = kPyCandidates[entry->offset + i];
        ctx->cand_count = vis;
        return IME_CANDIDATES;
    }

    /* Prefix search: check if any table entry starts with ctx->preedit */
    bool has_prefix = false;
    /* Simple linear scan over the ~334 entries to check prefix validity.
     * This is O(334) string comparisons in MCU Flash — acceptable since
     * preedit changes happen at human speed (not per frame). */
    for (uint16_t i = 0; i < kPyTableLen; i++) {
        const char *syl = kPyTable[i].syllable;
        bool match = true;
        for (uint8_t j = 0; j < ctx->preedit_len; j++) {
            if (syl[j] != ctx->preedit[j]) { match = false; break; }
        }
        if (match && syl[0] != '\0') { has_prefix = true; break; }
    }

    if (!has_prefix) {
        ctx->no_match = true;
        return IME_PREEDIT;
    }
    return IME_PREEDIT;
}

/* Reload the visible candidate window from current cand_offset. */
static void s_reload_candidates(ImeCtx_t *ctx)
{
    const PinyinEntry_t *entry = PY_BSearch(ctx->preedit);
    if (!entry) { ctx->cand_count = 0; return; }

    uint8_t remain = (uint8_t)(entry->count - ctx->cand_offset);
    uint8_t vis    = (remain < IME_CANDS_VISIBLE) ? remain : IME_CANDS_VISIBLE;
    for (uint8_t i = 0; i < vis; i++)
        ctx->candidates[i] = kPyCandidates[entry->offset + ctx->cand_offset + i];
    ctx->cand_count = vis;
    ctx->cand_total = entry->count;
}

/* -------------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------- */

void IME_Reset(ImeCtx_t *ctx)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->mode = IME_MODE_LOWER;
}

ImeMode_t IME_CycleMode(ImeCtx_t *ctx)
{
    /* Clear any in-progress input when switching modes */
    memset(ctx->preedit,      0, sizeof(ctx->preedit));
    memset(ctx->letters_buf,  0, sizeof(ctx->letters_buf));
    ctx->preedit_len   = 0;
    ctx->letters_count = 0;
    ctx->cand_count    = 0;
    ctx->no_match      = false;

    ctx->mode = (ImeMode_t)((ctx->mode + 1u) % IME_MODE_COUNT);
    return ctx->mode;
}

ImeState_t IME_Feed(ImeCtx_t *ctx, KEY_Code_t key, bool held)
{
    (void)held;

    /* --- KEY_STAR: cycle input mode --- */
    if (key == KEY_STAR) {
        IME_CycleMode(ctx);
        return IME_IDLE;
    }

    /* --- KEY_EXIT: discard pending preedit / letter selection --- */
    if (key == KEY_EXIT) {
        if (ctx->preedit_len > 0 || ctx->letters_count > 0 || ctx->cand_count > 0) {
            memset(ctx->preedit,     0, sizeof(ctx->preedit));
            ctx->preedit_len   = 0;
            ctx->letters_count = 0;
            ctx->cand_count    = 0;
            ctx->no_match      = false;
            return IME_IDLE;
        }
        return IME_IDLE; /* not consumed — caller handles exit */
    }

    /* ================================================================
     * NON-PINYIN MODES: LOWER / UPPER / DIGIT
     * ============================================================== */
    if (ctx->mode != IME_MODE_PINYIN) {
        /* In non-pinyin modes the IME just signals IDLE so the caller
         * handles the key directly (cycle letters, digit, etc.). */
        return IME_IDLE;
    }

    /* ================================================================
     * PINYIN MODE
     * ============================================================== */

    /* --- Currently showing Unicode candidates --- */
    if (ctx->cand_count > 0) {
        if (key >= KEY_1 && key <= KEY_6) {
            uint8_t idx = (uint8_t)(key - KEY_1);
            if (idx < ctx->cand_count) {
                /* Commit: caller collects the char via IME_PickCandidate */
                return IME_CANDIDATES;  /* caller calls IME_PickCandidate() */
            }
        }
        if (key == KEY_UP || key == KEY_DOWN) {
            int8_t dir = (key == KEY_DOWN) ? 1 : -1;
            IME_ScrollCandidates(ctx, dir);
            return IME_CANDIDATES;
        }
        /* KEY_0 = cancel candidate selection, go back to preedit */
        if (key == KEY_0) {
            ctx->cand_count = 0;
            return IME_PREEDIT;
        }
        /* Any 2-9 key: start a new letter selection for the next char */
        if (key >= KEY_2 && key <= KEY_9) {
            /* Commit nothing — let caller handle; for now go to LETTERS */
            ctx->cand_count = 0;
            memset(ctx->preedit, 0, sizeof(ctx->preedit));
            ctx->preedit_len = 0;
            s_set_letters(ctx, key);
            return IME_LETTERS;
        }
        return IME_CANDIDATES; /* consume unknown key in candidate view */
    }

    /* --- Currently showing letter candidates (disambiguation) --- */
    if (ctx->letters_count > 0) {
        if (key >= KEY_1 && key <= KEY_4) {
            uint8_t idx = (uint8_t)(key - KEY_1);
            if (idx < ctx->letters_count) {
                char letter = ctx->letters_buf[idx];
                /* In UPPER mode, shift the letter */
                if (ctx->mode == IME_MODE_UPPER && letter >= 'a' && letter <= 'z')
                    letter = (char)(letter - 32);
                return s_add_to_preedit(ctx, letter);
            }
        }
        /* Same or different T9 key: replace letter set */
        if (key >= KEY_2 && key <= KEY_9) {
            s_set_letters(ctx, key);
            return IME_LETTERS;
        }
        /* KEY_0 = cancel letter selection */
        if (key == KEY_0) {
            ctx->letters_count = 0;
            return (ctx->preedit_len > 0) ? IME_PREEDIT : IME_IDLE;
        }
        return IME_LETTERS;
    }

    /* --- Preedit growing (no candidate window open) --- */
    if (ctx->preedit_len > 0) {
        /* T9 key: show letters */
        if (key >= KEY_2 && key <= KEY_9) {
            s_set_letters(ctx, key);
            return IME_LETTERS;
        }
        /* KEY_0 = backspace preedit */
        if (key == KEY_0) {
            IME_Backspace(ctx);
            return (ctx->preedit_len > 0) ? IME_PREEDIT : IME_IDLE;
        }
        return IME_PREEDIT;
    }

    /* --- IDLE in PINYIN mode: wait for first key --- */
    if (key >= KEY_2 && key <= KEY_9) {
        s_set_letters(ctx, key);
        return IME_LETTERS;
    }

    /* All other keys: pass through to caller */
    return IME_IDLE;
}

uint16_t IME_PickCandidate(ImeCtx_t *ctx, uint8_t idx)
{
    if (idx >= ctx->cand_count) return 0u;
    uint16_t cp = ctx->candidates[idx];
    /* Reset preedit and candidates */
    memset(ctx->preedit,    0, sizeof(ctx->preedit));
    ctx->preedit_len   = 0;
    ctx->cand_count    = 0;
    ctx->cand_offset   = 0;
    ctx->cand_total    = 0;
    ctx->no_match      = false;
    return cp;
}

uint8_t IME_ScrollCandidates(ImeCtx_t *ctx, int8_t direction)
{
    if (ctx->cand_total == 0) return 0;
    uint8_t new_off = (uint8_t)((int)ctx->cand_offset + direction * IME_CANDS_VISIBLE);
    if ((int)new_off < 0)
        new_off = 0;
    if (new_off >= ctx->cand_total)
        new_off = (uint8_t)(ctx->cand_total - 1u);
    /* Snap to IME_CANDS_VISIBLE boundary */
    new_off = (uint8_t)((new_off / IME_CANDS_VISIBLE) * IME_CANDS_VISIBLE);
    ctx->cand_offset = new_off;
    s_reload_candidates(ctx);
    return new_off;
}

bool IME_Backspace(ImeCtx_t *ctx)
{
    if (ctx->cand_count > 0 || ctx->letters_count > 0) {
        ctx->cand_count    = 0;
        ctx->letters_count = 0;
        if (ctx->preedit_len > 0)
            return true;  /* still in preedit */
        return true;
    }
    if (ctx->preedit_len > 0) {
        ctx->preedit[--ctx->preedit_len] = '\0';
        ctx->no_match = false;
        /* Re-check if the shorter preedit still has candidates */
        if (ctx->preedit_len > 0) {
            const PinyinEntry_t *entry = PY_BSearch(ctx->preedit);
            if (entry) {
                ctx->cand_offset = 0;
                s_reload_candidates(ctx);
            }
        }
        return true;
    }
    return false; /* caller should delete from edit buffer */
}

const char *IME_ModeLabel(const ImeCtx_t *ctx)
{
    switch (ctx->mode) {
    case IME_MODE_LOWER:  return "abc";
    case IME_MODE_UPPER:  return "ABC";
    case IME_MODE_DIGIT:  return "123";
    case IME_MODE_PINYIN: return "\xe6\x8b\xbc"; /* 拼 */
    default:              return "?";
    }
}
