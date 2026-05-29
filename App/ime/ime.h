/* SPDX-License-Identifier: Apache-2.0
 * Pinyin IME state machine — Quansheng-style two-row interface.
 *
 * Design (Quansheng UV-K5 style):
 *  1. Press T9 key(s) → option row shows valid pinyin prefixes derived from
 *     all letter combinations; candidate row shows chars for first prefix.
 *  2. KEY_UP cycles to next prefix option (candidate row updates).
 *  3. KEY_MENU moves focus from option row → candidate row.
 *  4. KEY_1..KEY_6 (in candidate focus) commits a character.
 *  5. KEY_UP/DOWN in candidate focus pages through candidates.
 *  6. KEY_EXIT removes last T9 key from sequence (or exits candidate focus).
 *
 * Example — press key 6 (mno):
 *   Option row  :  >m n o          (focus on 'm', others shown after)
 *   Candidate row:  1面 2命 3马 4梦 5木    (candidates for prefix 'm')
 *   → KEY_UP → option row: >n o  / candidates for 'n'
 *   → KEY_MENU → option row: n o  / >1你 2年 3呢 ...  (candidate focus)
 *   → KEY_1 → commits 你
 *
 * Example — press key 6 then 4 (mno × ghi):
 *   Option row  :  >mi ni          (only valid syllable combos shown)
 *   Candidate row:  1米 2迷 3蜜 4咪 5谜
 */

#ifndef IME_IME_H
#define IME_IME_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/keyboard.h"

/* -------------------------------------------------------------------------
 * IME states (return value of IME_Feed)
 * --------------------------------------------------------------------- */
typedef enum {
    IME_IDLE       = 0, /* no active PY input; key not consumed by IME     */
    IME_OPTION,         /* option row has focus (T9 keys extend sequence)  */
    IME_CANDIDATES,     /* candidate row has focus (1-6 commit a char)     */
    IME_REJECTED,       /* key produced no valid pinyin; state unchanged   */
} ImeState_t;

/* Input mode (cycled by KEY_F / #) */
typedef enum {
    IME_MODE_EN     = 0, /* English T9 multi-tap */
    IME_MODE_PINYIN,     /* Chinese pinyin       */
    IME_MODE_COUNT
} ImeMode_t;

/* Max T9 keys that can be pressed in sequence */
#define IME_KEY_SEQ_MAX   6u
/* Max option strings derived from the key sequence */
#define IME_OPTS_MAX      16u
/* Max chars in one option string (longest syllable "zhuang" = 6 + NUL) */
#define IME_OPT_STR_MAX   8u
/* Candidates shown at once (KEY_1 … KEY_5) */
#define IME_CANDS_VISIBLE 5u

/* -------------------------------------------------------------------------
 * Context — one per active editor, caller-owned
 * --------------------------------------------------------------------- */
typedef struct {
    ImeMode_t  mode;

    /* --- PY key sequence (T9 keys pressed so far) --- */
    uint8_t    py_keys[IME_KEY_SEQ_MAX]; /* KEY_2 … KEY_9 in order          */
    uint8_t    py_key_count;             /* valid entries in py_keys[]       */

    /* --- Option list (valid pinyin prefixes from py_keys combos) --- */
    char       py_opts[IME_OPTS_MAX][IME_OPT_STR_MAX]; /* prefix strings    */
    uint8_t    py_opt_count;             /* number of valid options          */
    uint8_t    py_opt_idx;               /* currently highlighted option     */

    /* --- Candidate state --- */
    bool       cand_focus;               /* false=option row, true=cand row  */
    uint8_t    cand_page;                /* which page of candidates (0-based)*/
    bool       has_more;                 /* more candidates beyond this page  */

    uint16_t   candidates[IME_CANDS_VISIBLE]; /* Unicode codepoints          */
    uint8_t    cand_count;               /* valid entries in candidates[]     */
} ImeCtx_t;

/* -------------------------------------------------------------------------
 * API
 * --------------------------------------------------------------------- */

/* (Re)initialise context to EN mode, all PY state cleared. */
void IME_Reset(ImeCtx_t *ctx);

/* Cycle input mode EN → PY → EN.  Clears all PY state.  Returns new mode. */
ImeMode_t IME_CycleMode(ImeCtx_t *ctx);

/* Feed one key press to the IME (non-EN modes only; EN handled by caller).
 * Returns:
 *   IME_OPTION     → option row updated; refresh display
 *   IME_CANDIDATES → candidate row active; refresh display
 *   IME_IDLE       → key NOT consumed by IME; caller must handle it
 *
 * Keys consumed internally:
 *   KEY_F  (#)    : cycle mode (always consumed)
 *   KEY_2–KEY_9   : extend key sequence (in PY option mode)
 *   KEY_UP/DOWN   : cycle options (option focus) or page candidates (cand focus)
 *   KEY_MENU      : toggle focus option↔candidate (only consumed when PY active)
 *   KEY_1–KEY_6   : select candidate index (only consumed in candidate focus)
 */
ImeState_t IME_Feed(ImeCtx_t *ctx, KEY_Code_t key, bool held);

/* In IME_CANDIDATES: pick the candidate at visible index idx (0-based).
 * Returns the Unicode codepoint or 0 if idx out of range.
 * Resets PY key sequence and candidate state (returns to IDLE). */
uint16_t IME_PickCandidate(ImeCtx_t *ctx, uint8_t idx);

/* Backspace one unit:
 *   cand_focus=true → exit candidate focus, stay in option mode (returns true)
 *   py_key_count>0  → remove last T9 key, rebuild options (returns true)
 *   otherwise       → returns false (caller should delete from edit buffer)
 */
bool IME_Backspace(ImeCtx_t *ctx);

/* Returns mode label: "EN" or "PY" */
const char *IME_ModeLabel(const ImeCtx_t *ctx);

#endif /* IME_IME_H */
