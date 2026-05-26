/* SPDX-License-Identifier: Apache-2.0
 * Runtime language accessor for Bafang firmware.
 *
 * g_lang is initialised from gEeprom.LANGUAGE in SETTINGS_InitEEPROM().
 * Default is LANG_EN so the firmware works before any font is written
 * to SPI Flash.                                                              */

#include "strings.h"
#include <stddef.h>

LangId_t g_lang = LANG_EN;

const char *STR(StrId_t id)
{
    if ((unsigned)id >= STR_COUNT)
        return "?";

    /* Use Chinese string only if:
     *   a) Chinese language is selected
     *   b) The entry is non-NULL and non-empty
     * Otherwise fall through to English.
     *
     * NOTE: Displaying a Chinese string requires the CJK glyph database
     * to be present in SPI Flash.  The caller (render layer) is responsible
     * for falling back to the Latin representation of each glyph when the
     * database is absent — STR() itself always returns the requested text.  */
    if (g_lang == LANG_ZH) {
        const char *s = g_str_zh[id];
        if (s && s[0] != '\0')
            return s;
    }

    return g_str_en[id];
}
