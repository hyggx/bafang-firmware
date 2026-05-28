/* SPDX-License-Identifier: Apache-2.0
 * Pinyin IME — Quansheng-style two-row state machine.
 *
 * Flow:
 *  1. T9 key(s) pressed → generate all letter combinations → filter to
 *     valid pinyin prefixes → store as py_opts[], load candidates for [0].
 *  2. KEY_UP  → cycle py_opt_idx, reload candidates.
 *  3. KEY_MENU → set cand_focus=true (focus moves to candidate row).
 *  4. KEY_1-6 with cand_focus → caller picks candidate via IME_PickCandidate.
 *  5. KEY_UP/DOWN with cand_focus → page through candidates.
 *  6. KEY_EXIT → IME_Backspace() pops last key or exits cand_focus.
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

/* Returns the index in kPyTable of the first entry whose syllable starts
 * with `prefix` (length `plen`), or kPyTableLen if none found.
 * Uses binary search since the table is sorted. */
static uint16_t s_prefix_first(const char *prefix, uint8_t plen)
{
    int lo = 0;
    int hi = (int)kPyTableLen - 1;
    int found = (int)kPyTableLen;
    while (lo <= hi) {
        int mid = lo + ((hi - lo) >> 1);
        /* Compare mid's syllable prefix */
        int cmp = 0;
        for (uint8_t j = 0; j < plen; j++) {
            cmp = (int)(uint8_t)kPyTable[mid].syllable[j] - (int)(uint8_t)prefix[j];
            if (cmp != 0) break;
        }
        if (cmp == 0) { found = mid; hi = mid - 1; } /* may be leftmost */
        else if (cmp < 0) lo = mid + 1;
        else              hi = mid - 1;
    }
    /* Verify the found position actually has this prefix */
    if (found < (int)kPyTableLen) {
        for (uint8_t j = 0; j < plen; j++) {
            if (kPyTable[found].syllable[j] != prefix[j])
                return (uint16_t)kPyTableLen;
        }
    }
    return (uint16_t)found;
}

/* Returns true if at least one kPyTable entry starts with prefix. */
static bool s_is_valid_prefix(const char *prefix, uint8_t plen)
{
    return s_prefix_first(prefix, plen) < kPyTableLen;
}

/* Load IME_CANDS_VISIBLE candidates for the current option (py_opts[py_opt_idx])
 * starting from page cand_page.  Fills ctx->candidates[], cand_count, has_more. */
static void s_load_cands_page(ImeCtx_t *ctx)
{
    ctx->cand_count = 0;
    ctx->has_more   = false;

    if (ctx->py_opt_count == 0)
        return;

    const char *prefix = ctx->py_opts[ctx->py_opt_idx];
    uint8_t     plen   = (uint8_t)__builtin_strlen(prefix);
    uint16_t    si     = s_prefix_first(prefix, plen);

    if (si >= kPyTableLen)
        return;

    /* Skip cand_page * IME_CANDS_VISIBLE candidates from the prefix range */
    uint16_t skip = (uint16_t)(ctx->cand_page * (uint16_t)IME_CANDS_VISIBLE);
    uint8_t  off  = 0;
    while (skip > 0 && si < kPyTableLen) {
        /* Check prefix still matches */
        bool still = true;
        for (uint8_t j = 0; j < plen; j++) {
            if (kPyTable[si].syllable[j] != prefix[j]) { still = false; break; }
        }
        if (!still) break;

        uint8_t avail = (uint8_t)(kPyTable[si].count - off);
        if ((uint16_t)avail <= skip) {
            skip -= (uint16_t)avail;
            si++;
            off = 0;
        } else {
            off = (uint8_t)(off + (uint8_t)skip);
            skip = 0;
        }
    }
    if (skip > 0)
        return; /* page beyond end */

    /* Collect up to IME_CANDS_VISIBLE candidates */
    uint8_t  n     = 0;
    uint16_t cur_si = si;
    uint8_t  cur_off = off;
    while (n < IME_CANDS_VISIBLE && cur_si < kPyTableLen) {
        bool still = true;
        for (uint8_t j = 0; j < plen; j++) {
            if (kPyTable[cur_si].syllable[j] != prefix[j]) { still = false; break; }
        }
        if (!still) break;

        while (cur_off < kPyTable[cur_si].count && n < IME_CANDS_VISIBLE)
            ctx->candidates[n++] = kPyCandidates[kPyTable[cur_si].offset + cur_off++];

        if (cur_off >= kPyTable[cur_si].count) { cur_si++; cur_off = 0; }
    }
    ctx->cand_count = n;

    /* Check for more candidates */
    if (n == IME_CANDS_VISIBLE && cur_si < kPyTableLen) {
        bool still = true;
        for (uint8_t j = 0; j < plen; j++) {
            if (kPyTable[cur_si].syllable[j] != prefix[j]) { still = false; break; }
        }
        ctx->has_more = still;
    }
}

/* Rebuild the option list from the current key sequence.
 * Uses iterative mixed-radix enumeration of all letter combinations and
 * filters to valid pinyin prefixes. */
static void s_rebuild_opts(ImeCtx_t *ctx)
{
    ctx->py_opt_count = 0;
    ctx->py_opt_idx   = 0;

    if (ctx->py_key_count == 0)
        return;

    /* Bounds check: all keys must be KEY_2..KEY_9 */
    for (uint8_t k = 0; k < ctx->py_key_count; k++) {
        if (ctx->py_keys[k] < KEY_2 || ctx->py_keys[k] > KEY_9)
            return;
    }

    /* letter_idx[i] = index into kT9Letters for key i */
    uint8_t letter_idx[IME_KEY_SEQ_MAX] = {0};

    while (ctx->py_opt_count < IME_OPTS_MAX) {
        /* Build current combo */
        char combo[IME_OPT_STR_MAX];
        for (uint8_t i = 0; i < ctx->py_key_count; i++)
            combo[i] = kT9Letters[ctx->py_keys[i] - KEY_2][letter_idx[i]];
        combo[ctx->py_key_count] = '\0';

        if (s_is_valid_prefix(combo, ctx->py_key_count)) {
            memcpy(ctx->py_opts[ctx->py_opt_count], combo, ctx->py_key_count + 1u);
            ctx->py_opt_count++;
        }

        /* Increment mixed-radix counter (rightmost digit first) */
        int8_t pos = (int8_t)(ctx->py_key_count - 1);
        while (pos >= 0) {
            uint8_t ki = ctx->py_keys[pos] - KEY_2;
            uint8_t nl = (uint8_t)__builtin_strlen(kT9Letters[ki]);
            if (++letter_idx[pos] < nl)
                break;
            letter_idx[pos] = 0;
            pos--;
        }
        if (pos < 0)
            break; /* all combinations exhausted */
    }
}

/* -------------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------- */

void IME_Reset(ImeCtx_t *ctx)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->mode = IME_MODE_EN;
}

ImeMode_t IME_CycleMode(ImeCtx_t *ctx)
{
    /* Clear all PY state when switching modes */
    ctx->py_key_count = 0;
    ctx->py_opt_count = 0;
    ctx->py_opt_idx   = 0;
    ctx->cand_focus   = false;
    ctx->cand_page    = 0;
    ctx->has_more     = false;
    ctx->cand_count   = 0;

    ctx->mode = (ImeMode_t)((ctx->mode + 1u) % IME_MODE_COUNT);
    return ctx->mode;
}

ImeState_t IME_Feed(ImeCtx_t *ctx, KEY_Code_t key, bool held)
{
    (void)held;

    /* KEY_F (#): cycle input mode — always consumed */
    if (key == KEY_F) {
        IME_CycleMode(ctx);
        return IME_IDLE;
    }

    /* Only handle PY mode below */
    if (ctx->mode != IME_MODE_PINYIN)
        return IME_IDLE;

    /* ================================================================
     * CANDIDATE FOCUS (after MENU was pressed)
     * ============================================================== */
    if (ctx->cand_focus) {
        /* KEY_1-5: signal "candidate N selected" to caller */
        if (key >= KEY_1 && key <= KEY_5) {
            uint8_t idx = (uint8_t)(key - KEY_1);
            if (idx < ctx->cand_count)
                return IME_CANDIDATES; /* caller calls IME_PickCandidate */
        }
        /* KEY_UP: next page */
        if (key == KEY_UP) {
            if (ctx->has_more) {
                ctx->cand_page++;
                s_load_cands_page(ctx);
            }
            return IME_CANDIDATES;
        }
        /* KEY_DOWN: prev page */
        if (key == KEY_DOWN) {
            if (ctx->cand_page > 0) {
                ctx->cand_page--;
                s_load_cands_page(ctx);
            }
            return IME_CANDIDATES;
        }
        /* KEY_EXIT or KEY_MENU: exit candidate focus → back to option row */
        if (key == KEY_EXIT || key == KEY_MENU) {
            ctx->cand_focus = false;
            return (ctx->py_opt_count > 0) ? IME_OPTION : IME_IDLE;
        }
        return IME_CANDIDATES; /* consume all other keys in cand focus */
    }

    /* ================================================================
     * OPTION FOCUS (T9 keys build up key sequence)
     * ============================================================== */

    /* T9 key: extend key sequence, rebuild options */
    if (key >= KEY_2 && key <= KEY_9) {
        if (ctx->py_key_count < IME_KEY_SEQ_MAX) {
            ctx->py_keys[ctx->py_key_count++] = key;
            ctx->cand_page = 0;
            s_rebuild_opts(ctx);
            if (ctx->py_opt_count > 0)
                s_load_cands_page(ctx);
            else
                ctx->cand_count = 0;
        }
        return (ctx->py_opt_count > 0) ? IME_OPTION : IME_IDLE;
    }

    /* KEY_UP: cycle to next option */
    if (key == KEY_UP && ctx->py_opt_count > 1) {
        ctx->py_opt_idx = (uint8_t)((ctx->py_opt_idx + 1u) % ctx->py_opt_count);
        ctx->cand_page  = 0;
        s_load_cands_page(ctx);
        return IME_OPTION;
    }

    /* KEY_DOWN: cycle to previous option */
    if (key == KEY_DOWN && ctx->py_opt_count > 1) {
        ctx->py_opt_idx = (uint8_t)(
            (ctx->py_opt_idx + ctx->py_opt_count - 1u) % ctx->py_opt_count);
        ctx->cand_page = 0;
        s_load_cands_page(ctx);
        return IME_OPTION;
    }

    /* KEY_MENU: move focus to candidate row (only if candidates exist) */
    if (key == KEY_MENU && ctx->cand_count > 0) {
        ctx->cand_focus = true;
        return IME_CANDIDATES;
    }

    /* KEY_UP/DOWN with only 1 option: still consume to avoid cursor moving */
    if ((key == KEY_UP || key == KEY_DOWN) && ctx->py_opt_count == 1)
        return IME_OPTION;

    /* Stay in OPTION state for any unhandled key if we have active options */
    if (ctx->py_opt_count > 0 || ctx->py_key_count > 0)
        return IME_OPTION;

    return IME_IDLE;
}

uint16_t IME_PickCandidate(ImeCtx_t *ctx, uint8_t idx)
{
    if (idx >= ctx->cand_count) return 0u;
    uint16_t cp = ctx->candidates[idx];
    /* Reset all PY state */
    ctx->py_key_count = 0;
    ctx->py_opt_count = 0;
    ctx->py_opt_idx   = 0;
    ctx->cand_focus   = false;
    ctx->cand_page    = 0;
    ctx->has_more     = false;
    ctx->cand_count   = 0;
    return cp;
}

bool IME_Backspace(ImeCtx_t *ctx)
{
    /* Exit candidate focus without clearing anything */
    if (ctx->cand_focus) {
        ctx->cand_focus = false;
        return true;
    }
    /* Remove last T9 key from sequence */
    if (ctx->py_key_count > 0) {
        ctx->py_key_count--;
        ctx->cand_page = 0;
        if (ctx->py_key_count > 0) {
            s_rebuild_opts(ctx);
            if (ctx->py_opt_count > 0)
                s_load_cands_page(ctx);
            else
                ctx->cand_count = 0;
        } else {
            ctx->py_opt_count = 0;
            ctx->cand_count   = 0;
        }
        return true;
    }
    return false; /* caller should delete from edit buffer */
}

const char *IME_ModeLabel(const ImeCtx_t *ctx)
{
    switch (ctx->mode) {
    case IME_MODE_EN:     return "EN";
    case IME_MODE_PINYIN: return "PY";
    default:              return "?";
    }
}

