/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include <string.h>
#include <stdlib.h>

#include "../app/dtmf.h"
#include "../app/menu.h"
#include "../bitmaps.h"
#include "../board.h"
#include "../dcs.h"
#include "../driver/backlight.h"
#include "../driver/bk4819.h"
#include "../driver/eeprom.h"
#include "../driver/st7565.h"
#include "../external/printf/printf.h"
#include "../font.h"
#include "../frequencies.h"
#include "../helper/battery.h"
#include "../misc.h"
#include "../settings.h"

#ifdef ENABLE_FEAT_F4HWN
    #include "../version.h"
#endif

#include "helper.h"
#include "inputbox.h"
#include "menu.h"
#include "../l10n/cjk_font.h"
#include "menu_sub_values_zh.h"
#include "ui.h"
#include "welcome.h"
#include "../l10n/strings.h"

/* Select between English and localised Chinese sub-menu value string.
 * en / zh are already-indexed string expressions (e.g. arr[i], or a literal).
 * Returns const char *. */
#define SUBV(en, zh) ((g_lang == LANG_ZH) ? (const char *)(zh) : (const char *)(en))

/* --------------------------------------------------------------------------
 * menu_id → StrId_t lookup table (256 bytes, flash).
 * All unmapped slots hold 0xFF (> STR_COUNT) so MENU_ItemName() falls back
 * to the ASCII .name field for any unknown/sentinel id.
 * Uses GCC designated-range initialisers which arm-none-eabi-gcc supports.
 * -------------------------------------------------------------------------- */
static const uint8_t kMenuIdToStr[256] = {
    [0 ... 255]          = 0xFFu,
    [MENU_SQL]           = STR_MENU_SQL,
    [MENU_STEP]          = STR_MENU_STEP,
    [MENU_TXP]           = STR_MENU_TXP,
    [MENU_R_DCS]         = STR_MENU_R_DCS,
    [MENU_R_CTCS]        = STR_MENU_R_CTCS,
    [MENU_T_DCS]         = STR_MENU_T_DCS,
    [MENU_T_CTCS]        = STR_MENU_T_CTCS,
    [MENU_SFT_D]         = STR_MENU_SFT_D,
    [MENU_OFFSET]        = STR_MENU_OFFSET,
    [MENU_TOT]           = STR_MENU_TOT,
    [MENU_W_N]           = STR_MENU_W_N,
#ifndef ENABLE_FEAT_F4HWN
    [MENU_SCR]           = STR_MENU_SCR,
#endif
    [MENU_BCL]           = STR_MENU_BCL,
#ifdef ENABLE_FEAT_F4HWN
    [MENU_TX_LOCK]       = STR_MENU_TX_LOCK,
#endif
    [MENU_MEM_CH]        = STR_MENU_MEM_CH,
    [MENU_DEL_CH]        = STR_MENU_DEL_CH,
    [MENU_MEM_NAME]      = STR_MENU_MEM_NAME,
    [MENU_MDF]           = STR_MENU_MDF,
    [MENU_SAVE]          = STR_MENU_SAVE,
    [MENU_VOX]           = STR_MENU_VOX,
    [MENU_ABR]           = STR_MENU_ABR,
    [MENU_ABR_ON_TX_RX]  = STR_MENU_ABR_ON_TX_RX,
    [MENU_ABR_MIN]       = STR_MENU_ABR_MIN,
    [MENU_ABR_MAX]       = STR_MENU_ABR_MAX,
    [MENU_TDR]           = STR_MENU_TDR,
    [MENU_BEEP]          = STR_MENU_BEEP,
#ifdef ENABLE_VOICE
    [MENU_VOICE]         = STR_MENU_VOICE,
#endif
    [MENU_SC_REV]        = STR_MENU_SC_REV,
    [MENU_AUTOLK]        = STR_MENU_AUTOLK,
    [MENU_LIST_CH]       = STR_MENU_LIST_CH,
    [MENU_STE]           = STR_MENU_STE,
    [MENU_RP_STE]        = STR_MENU_RP_STE,
    [MENU_MIC]           = STR_MENU_MIC,
    [MENU_MIC_BAR]       = STR_MENU_MIC_BAR,
    [MENU_COMPAND]       = STR_MENU_COMPAND,
    [MENU_1_CALL]        = STR_MENU_1_CALL,
    [MENU_S_LIST]        = STR_MENU_S_LIST,
    [MENU_S_PRI]         = STR_MENU_S_PRI,
    [MENU_S_PRI_CH_1]    = STR_MENU_S_PRI_CH_1,
    [MENU_S_PRI_CH_2]    = STR_MENU_S_PRI_CH_2,
#ifdef ENABLE_ALARM
    [MENU_AL_MOD]        = STR_MENU_AL_MOD,
#endif
#ifdef ENABLE_DTMF_CALLING
    [MENU_ANI_ID]        = STR_MENU_ANI_ID,
#endif
    [MENU_UPCODE]        = STR_MENU_UPCODE,
    [MENU_DWCODE]        = STR_MENU_DWCODE,
    [MENU_PTT_ID]        = STR_MENU_PTT_ID,
    [MENU_D_ST]          = STR_MENU_D_ST,
#ifdef ENABLE_DTMF_CALLING
    [MENU_D_RSP]         = STR_MENU_D_RSP,
    [MENU_D_HOLD]        = STR_MENU_D_HOLD,
#endif
    [MENU_D_PRE]         = STR_MENU_D_PRE,
#ifdef ENABLE_DTMF_CALLING
    [MENU_D_DCD]         = STR_MENU_D_DCD,
    [MENU_D_LIST]        = STR_MENU_D_LIST,
#endif
    [MENU_D_LIVE_DEC]    = STR_MENU_D_LIVE_DEC,
    [MENU_PONMSG]        = STR_MENU_PONMSG,
    [MENU_ROGER]         = STR_MENU_ROGER,
    [MENU_VOL]           = STR_MENU_VOL,
    [MENU_BAT_TXT]       = STR_MENU_BAT_TXT,
    [MENU_AM]            = STR_MENU_AM,
#ifndef ENABLE_FEAT_F4HWN
    #ifdef ENABLE_AM_FIX
    [MENU_AM_FIX]        = STR_MENU_AM_FIX,
    #endif
#endif
    [MENU_RESET]         = STR_MENU_RESET,
    [MENU_F_LOCK]        = STR_MENU_F_LOCK,
#ifndef ENABLE_FEAT_F4HWN
    [MENU_200TX]         = STR_MENU_200TX,
    [MENU_350TX]         = STR_MENU_350TX,
    [MENU_500TX]         = STR_MENU_500TX,
#endif
    [MENU_350EN]         = STR_MENU_350EN,
#ifndef ENABLE_FEAT_F4HWN
    [MENU_SCREN]         = STR_MENU_SCREN,
#endif
#ifdef ENABLE_F_CAL_MENU
    [MENU_F_CALI]        = STR_MENU_F_CALI,
#endif
#ifdef ENABLE_FEAT_F4HWN_SLEEP
    [MENU_SET_OFF]       = STR_MENU_SET_OFF,
#endif
#ifdef ENABLE_FEAT_F4HWN
    [MENU_SET_PTT]       = STR_MENU_SET_PTT,
    [MENU_SET_TOT]       = STR_MENU_SET_TOT,
    [MENU_SET_EOT]       = STR_MENU_SET_EOT,
    [MENU_SET_CTR]       = STR_MENU_SET_CTR,
    [MENU_SET_MET]       = STR_MENU_SET_MET,
    [MENU_SET_GUI]       = STR_MENU_SET_GUI,
    [MENU_SET_TMR]       = STR_MENU_SET_TMR,
    #ifdef ENABLE_FEAT_F4HWN_SCAN_FASTER
    [MENU_SET_SCN]       = STR_MENU_SET_SCN,
    #endif
    #ifdef ENABLE_FEAT_F4HWN_NARROWER
    [MENU_SET_NFM]       = STR_MENU_SET_NFM,
    #endif
    #ifdef ENABLE_FEAT_F4HWN_VOL
    [MENU_SET_VOL]       = STR_MENU_SET_VOL,
    #endif
    #ifdef ENABLE_FEAT_F4HWN_RESCUE_OPS
    [MENU_SET_KEY]       = STR_MENU_SET_KEY,
    #endif
    [MENU_SET_NAV]       = STR_MENU_SET_NAV,
    #ifdef ENABLE_FEAT_F4HWN_AUDIO
    [MENU_SET_AUD]       = STR_MENU_SET_AUD,
    #endif
#endif
    [MENU_BATCAL]        = STR_MENU_BATCAL,
    [MENU_F1SHRT]        = STR_MENU_F1SHRT,
    [MENU_F1LONG]        = STR_MENU_F1LONG,
    [MENU_F2SHRT]        = STR_MENU_F2SHRT,
    [MENU_F2LONG]        = STR_MENU_F2LONG,
    [MENU_MLONG]         = STR_MENU_MLONG,
    [MENU_BATTYP]        = STR_MENU_BATTYP,
    [MENU_LANG]          = STR_MENU_LANG,
};

/* Return the display name for a menu item: localised string if available,
 * otherwise the built-in ASCII .name field.                                 */
static inline const char *MENU_ItemName(const t_menu_item *item)
{
    if (g_lang != LANG_EN) {
        uint8_t sid = kMenuIdToStr[item->menu_id];
        if (sid < (uint8_t)STR_COUNT) {
            const char *s = STR((StrId_t)sid);
            if (s && s[0]) return s;
        }
    }
    return item->name;
}


const t_menu_item MenuList[] =
{
//   text,          menu ID               category
// ── 当前信道 (VFO) ─ per-channel saveable params ──────────────────────────
    {"Step",        MENU_STEP,          MENU_CAT_VFO},
    {"Power",       MENU_TXP,           MENU_CAT_VFO},    // was "TXP"
    {"W/N",         MENU_W_N,           MENU_CAT_VFO},
    {"Mode",        MENU_AM,            MENU_CAT_VFO},    // was "AM"
    {"RxDCS",       MENU_R_DCS,         MENU_CAT_VFO},    // was "R_DCS"
    {"RxCTCS",      MENU_R_CTCS,        MENU_CAT_VFO},    // was "R_CTCS"
    {"TxDCS",       MENU_T_DCS,         MENU_CAT_VFO},    // was "T_DCS"
    {"TxCTCS",      MENU_T_CTCS,        MENU_CAT_VFO},    // was "T_CTCS"
    {"TxODir",      MENU_SFT_D,         MENU_CAT_VFO},    // was "SFT_D"
    {"TxOffs",      MENU_OFFSET,        MENU_CAT_VFO},    // was "OFFSET"
    {"BusyCL",      MENU_BCL,           MENU_CAT_VFO},    // was "BCL"
    {"Compnd",      MENU_COMPAND,       MENU_CAT_VFO},
#ifndef ENABLE_FEAT_F4HWN
    {"Scramb",      MENU_SCR,           MENU_CAT_VFO},    // was "SCR"
#endif
#ifdef ENABLE_FEAT_F4HWN
    {"TXLock",      MENU_TX_LOCK,       MENU_CAT_VFO},
#endif
#ifdef ENABLE_FEAT_F4HWN_NARROWER
    {"SetNFM",      MENU_SET_NFM,       MENU_CAT_VFO},
#endif
#ifdef ENABLE_FEAT_F4HWN_AUDIO
    {"SetRxA",      MENU_SET_AUD,       MENU_CAT_VFO},
#endif
#ifdef ENABLE_FEAT_F4HWN_VOL
    {"SetRxGn",     MENU_SET_VOL,       MENU_CAT_VFO},
#endif

// ── 信道管理 (CHANMGR) ─ channel CRUD + scan lists ─────────────────────────
    {"ChList",      MENU_LIST_CH,       MENU_CAT_CHANMGR},
    {"ChSave",      MENU_MEM_CH,        MENU_CAT_CHANMGR}, // was "MEM-CH"
    {"ChName",      MENU_MEM_NAME,      MENU_CAT_CHANMGR},
    {"ChDele",      MENU_DEL_CH,        MENU_CAT_CHANMGR}, // was "DEL-CH"
    {"1 Call",      MENU_1_CALL,        MENU_CAT_CHANMGR},
    {"ScList",      MENU_S_LIST,        MENU_CAT_CHANMGR},
    {"ScPri",       MENU_S_PRI,         MENU_CAT_CHANMGR},
    {"PriCh1",      MENU_S_PRI_CH_1,    MENU_CAT_CHANMGR},
    {"PriCh2",      MENU_S_PRI_CH_2,    MENU_CAT_CHANMGR},
    {"ScnRev",      MENU_SC_REV,        MENU_CAT_CHANMGR},
#ifdef ENABLE_FEAT_F4HWN
    #ifdef ENABLE_FEAT_F4HWN_SCAN_FASTER
    {"SetScn",      MENU_SET_SCN,       MENU_CAT_CHANMGR},
    #endif
    #ifdef ENABLE_NOAA
    {"SetNWR",      MENU_NOAA_S,        MENU_CAT_CHANMGR},
    #endif
#else
    #ifdef ENABLE_NOAA
    {"NOAA-S",      MENU_NOAA_S,        MENU_CAT_CHANMGR},
    #endif
#endif

// ── 收发设置 (RADIO) ─ global TX/RX behaviour ────────────────────────────
    {"Sql",         MENU_SQL,           MENU_CAT_RADIO},
    {"RxMode",      MENU_TDR,           MENU_CAT_RADIO},
#ifdef ENABLE_VOX
    {"VOX",         MENU_VOX,           MENU_CAT_RADIO},
#endif
    {"TxTOut",      MENU_TOT,           MENU_CAT_RADIO},   // was "TOT"
    {"Mic",         MENU_MIC,           MENU_CAT_RADIO},
    {"Roger",       MENU_ROGER,         MENU_CAT_RADIO},
    {"STE",         MENU_STE,           MENU_CAT_RADIO},
    {"RP STE",      MENU_RP_STE,        MENU_CAT_RADIO},
#ifdef ENABLE_ALARM
    {"AlarmT",      MENU_AL_MOD,        MENU_CAT_RADIO},
#endif
#ifndef ENABLE_FEAT_F4HWN
    #ifdef ENABLE_AM_FIX
    {"AM Fix",      MENU_AM_FIX,        MENU_CAT_RADIO},
    #endif
#endif
#ifdef ENABLE_FEAT_F4HWN
    {"SetTOT",      MENU_SET_TOT,       MENU_CAT_RADIO},
    {"SetEOT",      MENU_SET_EOT,       MENU_CAT_RADIO},
#endif

// ── DTMF ─────────────────────────────────────────────────────────────────
    {"UPCode",      MENU_UPCODE,        MENU_CAT_DTMF},
    {"DWCode",      MENU_DWCODE,        MENU_CAT_DTMF},
    {"PTT ID",      MENU_PTT_ID,        MENU_CAT_DTMF},
    {"D ST",        MENU_D_ST,          MENU_CAT_DTMF},
    {"D Prel",      MENU_D_PRE,         MENU_CAT_DTMF},
    {"D Live",      MENU_D_LIVE_DEC,    MENU_CAT_DTMF},   // live DTMF decoder
#ifdef ENABLE_DTMF_CALLING
    {"ANI ID",      MENU_ANI_ID,        MENU_CAT_DTMF},
    {"D Resp",      MENU_D_RSP,         MENU_CAT_DTMF},
    {"D Hold",      MENU_D_HOLD,        MENU_CAT_DTMF},
    {"D Decd",      MENU_D_DCD,         MENU_CAT_DTMF},
    {"D List",      MENU_D_LIST,        MENU_CAT_DTMF},
#endif

// ── 界面按键 (UI) ─ display, backlight, keys, audio feedback ─────────────
    {"ChDisp",      MENU_MDF,           MENU_CAT_UI},     // was "MDF"
    {"POnMsg",      MENU_PONMSG,        MENU_CAT_UI},
    {"BatTxt",      MENU_BAT_TXT,       MENU_CAT_UI},
    {"BLTime",      MENU_ABR,           MENU_CAT_UI},     // was "ABR"
    {"BLMin",       MENU_ABR_MIN,       MENU_CAT_UI},
    {"BLMax",       MENU_ABR_MAX,       MENU_CAT_UI},
    {"BLTxRx",      MENU_ABR_ON_TX_RX,  MENU_CAT_UI},
#ifdef ENABLE_FEAT_F4HWN
    {"SetCtr",      MENU_SET_CTR,       MENU_CAT_UI},
    {"SetGUI",      MENU_SET_GUI,       MENU_CAT_UI},
    {"SetMet",      MENU_SET_MET,       MENU_CAT_UI},
#endif
    {"F1Shrt",      MENU_F1SHRT,        MENU_CAT_UI},
    {"F1Long",      MENU_F1LONG,        MENU_CAT_UI},
    {"F2Shrt",      MENU_F2SHRT,        MENU_CAT_UI},
    {"F2Long",      MENU_F2LONG,        MENU_CAT_UI},
    {"M Long",      MENU_MLONG,         MENU_CAT_UI},
    {"KeyLck",      MENU_AUTOLK,        MENU_CAT_UI},     // was "AUTOLk"
#ifdef ENABLE_FEAT_F4HWN
    {"SetPTT",      MENU_SET_PTT,       MENU_CAT_UI},
#endif
    {"Beep",        MENU_BEEP,          MENU_CAT_UI},
#ifdef ENABLE_VOICE
    {"Voice",       MENU_VOICE,         MENU_CAT_UI},
#endif
#ifdef ENABLE_AUDIO_BAR
    {"MicBar",      MENU_MIC_BAR,       MENU_CAT_UI},
#endif
#ifndef ENABLE_FEAT_F4HWN
    {"BatVol",      MENU_VOL,           MENU_CAT_UI},     // was "VOL"
#endif

// ── 系统设置 (SYSTEM) ────────────────────────────────────────────────────
    {"BatSav",      MENU_SAVE,          MENU_CAT_SYSTEM}, // was "SAVE"
#ifdef ENABLE_FEAT_F4HWN
    {"SetTmr",      MENU_SET_TMR,       MENU_CAT_SYSTEM},
    #ifdef ENABLE_FEAT_F4HWN_SLEEP
    {"SetOff",      MENU_SET_OFF,       MENU_CAT_SYSTEM},
    #endif
    #ifdef ENABLE_FEAT_F4HWN_RESCUE_OPS
    {"SetKey",      MENU_SET_KEY,       MENU_CAT_SYSTEM},
    #endif
    {"Lang",        MENU_LANG,          MENU_CAT_SYSTEM}, // UI language EN/ZH
    {"SysInf",      MENU_VOL,           MENU_CAT_SYSTEM}, // system info (last)
#endif

// ── HIDDEN ───────────────────────────────────────
// enabled if pressing both the PTT and upper side button at power-on
    {"F Lock",      MENU_F_LOCK,        MENU_CAT_NONE},
#ifndef ENABLE_FEAT_F4HWN
    {"Tx 200",      MENU_200TX,         MENU_CAT_NONE}, // was "200TX"
    {"Tx 350",      MENU_350TX,         MENU_CAT_NONE}, // was "350TX"
    {"Tx 500",      MENU_500TX,         MENU_CAT_NONE}, // was "500TX"
#endif
    {"350 En",      MENU_350EN,         MENU_CAT_NONE}, // was "350EN"
#ifndef ENABLE_FEAT_F4HWN
    {"ScraEn",      MENU_SCREN,         MENU_CAT_NONE}, // was "SCREN"
#endif
#ifdef ENABLE_F_CAL_MENU
    {"FrCali",      MENU_F_CALI,        MENU_CAT_NONE}, // reference xtal calibration
#endif
    {"BatCal",      MENU_BATCAL,        MENU_CAT_NONE}, // battery voltage calibration
    {"BatTyp",      MENU_BATTYP,        MENU_CAT_NONE}, // battery type 1600/2200mAh
    {"SetNav",      MENU_SET_NAV,       MENU_CAT_NONE}, // set navigation direction
    {"Reset",       MENU_RESET,         MENU_CAT_NONE}, // factory reset

    {"",            0xff,               MENU_CAT_NONE}  // end of list - DO NOT delete or move
};

const uint8_t FIRST_HIDDEN_MENU_ITEM = MENU_F_LOCK;

const char gSubMenu_TXP[][5] =
{
    "VLow",
    "Low",
    "Mid",
    "High"
};

const char gSubMenu_SFT_D[][4] =
{
    "OFF",
    "+",
    "-"
};

const char gSubMenu_W_N[][7] =
{
    "WIDE",
    "NARROW"
};

const char gSubMenu_OFF_ON[][4] =
{
    "OFF",
    "ON"
};

const char gSubMenu_NA[4] =
{
    "N/A"
};

const char* const gSubMenu_RXMode[] =
{
    "MAIN\nONLY",       // TX and RX on main only
    "DUAL RX\nRESPOND", // Watch both and respond
    "CROSS\nBAND",      // TX on main, RX on secondary
    "MAIN TX\nDUAL RX"  // always TX on main, but RX on both
};

#ifdef ENABLE_VOICE
    const char gSubMenu_VOICE[][4] =
    {
        "OFF",
        "CHI",
        "ENG"
    };
#endif

const char* const gSubMenu_MDF[] =
{
    "FREQ",
    "CHANNEL\nNUMBER",
    "NAME",
    "NAME\n+\nFREQ"
};

#ifdef ENABLE_ALARM
    const char gSubMenu_AL_MOD[][5] =
    {
        "SITE",
        "TONE"
    };
#endif

#ifdef ENABLE_DTMF_CALLING
const char gSubMenu_D_RSP[][11] =
{
    "DO\nNOTHING",
    "RING",
    "REPLY",
    "BOTH"
};
#endif

const char* const gSubMenu_PTT_ID[] =
{
    "OFF",
    "UP CODE",
    "DOWN CODE",
    "UP+DOWN\nCODE",
    "APOLLO\nQUINDAR"
};

const char gSubMenu_PONMSG[][8] =
{
#ifdef ENABLE_FEAT_F4HWN
    "ALL",
    "SOUND",
#else
    "FULL",
#endif
    "MESSAGE",
    "VOLTAGE",
#ifdef ENABLE_FEAT_F4HWN_LOGO
    "LOGO",
#endif
    "NONE"
};

const char gSubMenu_ROGER[][6] =
{
    "OFF",
    "ROGER",
    "MDC"
};

const char gSubMenu_RESET[][4] =
{
    "VFO",
    "ALL"
};

const char * const gSubMenu_F_LOCK[] =
{
    "DEFAULT+\n137-174\n400-470",
    "FCC HAM\n144-148\n420-450",
#ifdef ENABLE_FEAT_F4HWN_CA
    "CA HAM\n144-148\n430-450",
#endif
    "CE HAM\n144-146\n430-440",
    "GB HAM\n144-148\n430-440",
    "137-174\n400-430",
    "137-174\n400-438",
#ifdef ENABLE_FEAT_F4HWN_PMR
    "PMR 446",
#endif
#ifdef ENABLE_FEAT_F4HWN_GMRS_FRS_MURS
    "GMRS\nFRS\nMURS",
#endif
    "DISABLE\nALL",
    "UNLOCK\nALL",
};

const char gSubMenu_RX_TX[][6] =
{
    "OFF",
    "TX",
    "RX",
    "TX/RX"
};

const char gSubMenu_BAT_TXT[][8] =
{
    "NONE",
    "VOLTAGE",
    "PERCENT"
};

const char gSubMenu_BATTYP[][12] =
{
    "1600mAh K5",
    "2200mAh K5",
    "3500mAh K5",
    "1400mAh K1",
    "2500mAh K1"
};

const char gSubMenu_LANG[][4] =
{
    "EN",
    "ZH",
};

const char gSubMenu_SET_NAV[][17] =
{
    "LEFT\nRIGHT\nUV-K1",
    "UP\nDOWN\nUV-K5(8)",
};

#ifndef ENABLE_FEAT_F4HWN
const char gSubMenu_SCRAMBLER[][7] =
{
    "OFF",
    "2600Hz",
    "2700Hz",
    "2800Hz",
    "2900Hz",
    "3000Hz",
    "3100Hz",
    "3200Hz",
    "3300Hz",
    "3400Hz",
    "3500Hz"
};
#endif

#ifdef ENABLE_FEAT_F4HWN
    const char gSubMenu_SET_PTT[][8] =
    {
        "CLASSIC",
        "ONEPUSH"
    };

    const char gSubMenu_SET_TOT[][7] =  // Use by SET_EOT too
    {
        "OFF",
        "SOUND",
        "VISUAL",
        "ALL"
    };

    const char gSubMenu_SET_MET[][8] =
    {
        "TINY",
        "CLASSIC"
    };

    #ifdef ENABLE_FEAT_F4HWN_SCAN_FASTER
        const char gSubMenu_SET_SCN[][7] =
        {
            "NORMAL",
            "FAST"
        };
    #endif

    #ifdef ENABLE_FEAT_F4HWN_AUDIO
        const char gSubMenu_SET_AUD_FM[][6] =
        {
            "FLAT",
            "CLEAN",
            "MID",
            "BOOST",
            "MAX"
        };

        const char gSubMenu_SET_AUD_AM[][6] =
        {
            "SHARP",
            "STOCK",
            "OPEN"
        };
    #endif

    #ifdef ENABLE_FEAT_F4HWN_NARROWER
        const char gSubMenu_SET_NFM[][9] =
        {
            "NARROW",
            "NARROWER"
        };
    #endif

    #ifdef ENABLE_FEAT_F4HWN_RESCUE_OPS
        const char gSubMenu_SET_KEY[][9] =
        {
            "KEY_MENU",
            "KEY_UP",
            "KEY_DOWN",
            "KEY_EXIT",
            "KEY_STAR"
        };
    #endif
#endif

const t_sidefunction gSubMenu_SIDEFUNCTIONS[] =
{
    {"NONE",            ACTION_OPT_NONE},
#ifdef ENABLE_FLASHLIGHT
    {"FLASH\nLIGHT",    ACTION_OPT_FLASHLIGHT},
#endif
    {"POWER",           ACTION_OPT_POWER},
    {"MONITOR",         ACTION_OPT_MONITOR},
    {"SCAN",            ACTION_OPT_SCAN},
#ifdef ENABLE_VOX
    {"VOX",             ACTION_OPT_VOX},
#endif
#ifdef ENABLE_ALARM
    {"ALARM",           ACTION_OPT_ALARM},
#endif
#ifdef ENABLE_FMRADIO
    {"FM RADIO",        ACTION_OPT_FM},
#endif
#ifdef ENABLE_TX1750
    {"1750Hz",          ACTION_OPT_1750},
#endif
#ifdef ENABLE_REGA
    {"REGA\nALARM",     ACTION_OPT_REGA_ALARM},
    {"REGA\nTEST",      ACTION_OPT_REGA_TEST},
#endif
    {"LOCK\nKEYPAD",    ACTION_OPT_KEYLOCK},
    {"VFO A\nVFO B",    ACTION_OPT_A_B},
    {"VFO\nMEM",        ACTION_OPT_VFO_MR},
    {"MODE",            ACTION_OPT_SWITCH_DEMODUL},
#ifdef ENABLE_BLMIN_TMP_OFF
    {"BLMIN\nTMP OFF",  ACTION_OPT_BLMIN_TMP_OFF},      //BackLight Minimum Temporay OFF
#endif
#ifdef ENABLE_FEAT_F4HWN
    {"RX MODE",         ACTION_OPT_RXMODE},
    {"MAIN ONLY",       ACTION_OPT_MAINONLY},
    {"PTT",             ACTION_OPT_PTT},
    {"WIDE\nNARROW",    ACTION_OPT_WN},
    {"MUTE",            ACTION_OPT_MUTE},
    #ifdef ENABLE_FEAT_F4HWN_AUDIO
        {"RxA",            ACTION_OPT_RXA},
    #endif
    #ifdef ENABLE_FEAT_F4HWN_RESCUE_OPS
        {"POWER\nHIGH",    ACTION_OPT_POWER_HIGH},
        {"REMOVE\nOFFSET",  ACTION_OPT_REMOVE_OFFSET},
    #endif
    #ifdef ENABLE_FEAT_F4HWN_BEAM
        {"BEAM",            ACTION_OPT_BEAM},
    #endif
#endif
};

const uint8_t gSubMenu_SIDEFUNCTIONS_size = ARRAY_SIZE(gSubMenu_SIDEFUNCTIONS);

/* Render one sub-menu value line: uses UTF-8 renderer for CJK text with
 * simple pixel-width centering; falls back to the standard ASCII renderer
 * for pure-ASCII strings so there is no behaviour change in English mode. */
static void print_menu_value(const char *s,
                             unsigned int x1, unsigned int x2, unsigned int y)
{
    bool cjk = false;
    for (const char *p = s; *p; p++)
        if ((unsigned char)*p >= 0x80u) { cjk = true; break; }

    if (cjk) {
        /* Estimate pixel width: CJK multi-byte → 13 px, ASCII → 8 px. */
        uint8_t pw = 0;
        for (const uint8_t *p = (const uint8_t *)s; *p >= 0x20u; ) {
            if (*p >= 0xC0u) {
                pw += 13u;
                p += (*p < 0xE0u) ? 2u : (*p < 0xF0u) ? 3u : 4u;
            } else if (*p >= 0x80u) {
                p++; /* skip continuation byte */
            } else {
                pw += 8u; p++;
            }
        }
        uint8_t pane = (x2 > x1) ? (uint8_t)(x2 - x1 + 1u) : 0u;
        uint8_t cx   = (uint8_t)x1 + (pw < pane ? (pane - pw) / 2u : 0u);
        UI_PrintStringUTF8(s, cx, (uint8_t)y);
    } else {
        UI_PrintString(s, x1, x2, y, 8);
    }
}

bool    gIsInSubMenu;
uint8_t gMenuCursor;
uint8_t gMenuCategory   = MENU_CAT_NONE; // MENU_CAT_NONE = top-level category screen
uint8_t gMenuCatCursor  = 0;             // highlighted category at top level
uint8_t gMenuCatFirstIdx[MENU_CAT_COUNT];
uint8_t gMenuCatItemCount[MENU_CAT_COUNT];
uint8_t gMenuCatSavedCursor[MENU_CAT_COUNT]; // last cursor position per category (0xFF = unvisited)
int UI_MENU_GetCurrentMenuId() {
    if(gMenuCursor < ARRAY_SIZE(MenuList))
        return MenuList[gMenuCursor].menu_id;

    return MenuList[ARRAY_SIZE(MenuList)-1].menu_id;
}

uint8_t UI_MENU_GetMenuIdx(uint8_t id)
{
    for(uint8_t i = 0; i < ARRAY_SIZE(MenuList); i++)
        if(MenuList[i].menu_id == id)
            return i;
    return 0;
}

/* Navigate to menu_id and enter its category (no-op when gF_LOCK is set). */
void UI_MENU_GoToItem(uint8_t menu_id)
{
    gMenuCursor = UI_MENU_GetMenuIdx(menu_id);
    if (USE_CAT_MENU()) {
        uint8_t cat = MenuList[gMenuCursor].cat_id;
        if (cat < MENU_CAT_COUNT) {
            gMenuCategory  = cat;
            gMenuListCount = gMenuCatItemCount[cat];
        }
    }
}

int32_t gSubMenuSelection;

// edit box
char    edit_original[17]; // a copy of the text before editing so that we can easily test for changes/difference
char    edit[17];
int     edit_index;
bool    edit_is_uppercase = false;
ImeCtx_t g_ime;  /* pinyin IME context — reset each time channel-name editing starts */

/* -----------------------------------------------------------------------
 * Dedicated full-screen channel-name editor
 * Layout (128×64, 7 framebuffer lines × 8px + 1 status line):
 *
 *   gStatusLine      : mode label (ab/AB/12/PY) right-aligned
 *   gFrameBuffer[0]  : "CH NNN" small font, centred
 *   gFrameBuffer[1]  : entered chars top half (big/CJK font, lines 1-2)
 *   gFrameBuffer[2]  : entered chars bottom half
 *   gFrameBuffer[3]  : 10 slot underscores (bottom pixel of each slot)
 *   gFrameBuffer[4]  : cursor diamond (solid ♦, top of line)
 *   gFrameBuffer[5]  : preedit  ">pin"  (PY mode only)
 *   gFrameBuffer[6]  : candidates "1级2即..."  (PY mode only)
 *
 * Slot geometry: 10 slots × 8 px = 80 px, starting at x=24.
 *   ASCII/digit → 1 slot (7 px glyph, stride 8 px)
 *   CJK char    → 2 slots (12 px glyph centred in 16 px, +2 px left)
 *   Empty slot  → '-' dash placeholder (big font)
 * ----------------------------------------------------------------------- */
#define EDIT_SLOTS      10u
#define EDIT_SLOT_W      8u   /* big-font char stride (7 px glyph + 1 px gap) */
#define EDIT_START_X    ((LCD_WIDTH - EDIT_SLOTS * EDIT_SLOT_W) / 2u)  /* = 24 */

/* Return number of visual slots occupied by edit[0..byte_len). */
static uint8_t edit_count_slots(int byte_len)
{
    uint8_t slots = 0;
    int i = 0;
    while (i < byte_len) {
        uint8_t b = (uint8_t)edit[i];
        if (b >= 0xE0u) { slots += 2u; i += 3; }
        else if (b == ' ' || b == '\0') { break; }
        else { slots += 1u; i++; }
    }
    return slots;
}

static void UI_DisplayNameEdit(void)
{
    UI_DisplayClear();

    /* ---- line 0: IME mode label ("EN" / "PY") left-aligned ---- */
    {
        const char *label = IME_ModeLabel(&g_ime);
        UI_PrintStringSmallNormal(label, 0u, 0u, 0u);
    }

    /* ---- lines 1-2: edit buffer rendered as big font ---- */
    {
        static const unsigned int DASH_IDX = 12u; /* '-' - ' ' - 1 in gFontBig */
        uint8_t px    = EDIT_START_X;
        uint8_t slots = 0u;
        int     i     = 0;

        while (i < CHANNEL_NAME_MAX_BYTES && slots < EDIT_SLOTS) {
            uint8_t b = (uint8_t)edit[i];
            if (b == 0u)
                break;
            if (b >= 0xE0u) {
                /* CJK 3-byte UTF-8: 2 slots (16 px), glyph 12 px at +2 */
                char utf8[4] = { (char)b, edit[i + 1], edit[i + 2], '\0' };
                UI_PrintStringUTF8(utf8, (uint8_t)(px + 2u), 1u);
                px    += 2u * EDIT_SLOT_W;
                slots += 2u;
                i     += 3;
            } else if (b > (uint8_t)' ') {
                /* ASCII printable: 1 slot (8 px) */
                unsigned int idx = (unsigned int)b - (unsigned int)' ' - 1u;
                memcpy(gFrameBuffer[1] + px, &gFontBig[idx][0], 7u);
                memcpy(gFrameBuffer[2] + px, &gFontBig[idx][7], 7u);
                px    += EDIT_SLOT_W;
                slots += 1u;
                i++;
            } else {
                /* Space / unprintable → '-' dash placeholder */
                memcpy(gFrameBuffer[1] + px, &gFontBig[DASH_IDX][0], 7u);
                memcpy(gFrameBuffer[2] + px, &gFontBig[DASH_IDX][7], 7u);
                px    += EDIT_SLOT_W;
                slots += 1u;
                i++;
            }
        }
        /* Pad any remaining visual slots with dashes */
        while (slots < EDIT_SLOTS) {
            memcpy(gFrameBuffer[1] + px, &gFontBig[DASH_IDX][0], 7u);
            memcpy(gFrameBuffer[2] + px, &gFontBig[DASH_IDX][7], 7u);
            px    += EDIT_SLOT_W;
            slots++;
        }
    }

    /* ---- line 3: solid block cursor ----
     * Hidden when all 10 slots are filled (cur_slot == EDIT_SLOTS). */
    {
        uint8_t cur_slot = edit_count_slots(edit_index);
        if (cur_slot < EDIT_SLOTS) {
            uint8_t cx = (uint8_t)(EDIT_START_X + cur_slot * EDIT_SLOT_W);
            /* 4×4 diamond centred in the slot:
             *   row bit pattern (top→bottom): 0x04, 0x0E, 0x1F, 0x0E
             *   Stored LSB-first in a vertical byte: bit0=top row.
             *   Diamond occupies cols cx+2 .. cx+5 (4 px wide, centred in 8 px slot). */
            const uint8_t diamond[4] = { 0x04u, 0x0Eu, 0x1Fu, 0x0Eu };
            uint8_t mid = (uint8_t)(cx + EDIT_SLOT_W / 2u - 2u);
            for (uint8_t d = 0u; d < 4u && (uint8_t)(mid + d) < LCD_WIDTH; d++)
                gFrameBuffer[3][mid + d] = diamond[d];
        }
    }

    /* ---- PY mode overlays (lines 4-5): Quansheng-style two-row IME ---- */
    if (g_ime.mode == IME_MODE_PINYIN && g_ime.py_opt_count > 0u) {
        /* Line 4: option row
         *  cand_focus=false → ">opt[idx] opt[idx+1] ..."
         *  cand_focus=true  → " opt[idx]"  (just current option, no >)
         *  ">" is printed separately so option text always starts at OPT_START_X. */
        {
            const uint8_t OPT_START_X = 2u + 7u;  /* 2px margin + 7px reserved for ">" */
            if (!g_ime.cand_focus) {
                UI_PrintStringSmallNormal(">", 2u, 0u, 4u);
            }
            char obuf[32];
            uint8_t bpos = 0u;
            for (uint8_t i = g_ime.py_opt_idx;
                 i < g_ime.py_opt_count && bpos < (uint8_t)(sizeof(obuf) - 2u);
                 i++)
            {
                if (i > g_ime.py_opt_idx)
                    obuf[bpos++] = ' ';
                uint8_t ol = (uint8_t)__builtin_strlen(g_ime.py_opts[i]);
                if (bpos + ol >= (uint8_t)(sizeof(obuf) - 1u)) break;
                __builtin_memcpy(obuf + bpos, g_ime.py_opts[i], ol);
                bpos = (uint8_t)(bpos + ol);
                if (g_ime.cand_focus) break; /* show only current option */
            }
            obuf[bpos] = '\0';
            UI_PrintStringSmallNormal(obuf, OPT_START_X, 0u, 4u);
        }

        /* Line 5: candidate row
         *  cand_focus=false → " 1面 2命 3马 ..."  (indicator column blank, candidates at fixed x)
         *  cand_focus=true  → ">1面 2命 3马 ..."  (focus marker, candidates at same fixed x) */
        if (g_ime.cand_count > 0u) {
            const uint8_t CAND_START_X = 2u + 7u;  /* 2px margin + 7px reserved for ">" */
            if (g_ime.cand_focus) {
                UI_PrintStringSmallNormal(">", 2u, 0u, 5u);
            }
            uint8_t cx2 = CAND_START_X;
            char nbuf[2] = "1";
            for (uint8_t ci = 0u; ci < g_ime.cand_count && cx2 < 110u; ci++) {
                nbuf[0] = (char)('1' + ci);
                UI_PrintStringSmallNormal(nbuf, cx2, 0u, 5u);
                cx2 += 7u;
                char utf8[4];
                uint16_t cp = g_ime.candidates[ci];
                utf8[0] = (char)(0xE0u | (cp >> 12));
                utf8[1] = (char)(0x80u | ((cp >> 6) & 0x3Fu));
                utf8[2] = (char)(0x80u | (cp & 0x3Fu));
                utf8[3] = '\0';
                UI_PrintStringUTF8(utf8, cx2, 5u);
                cx2 += (uint8_t)(CJK_GLYPH_W + 2u); /* 12px + 2px gap = 14 */
            }
            if (g_ime.has_more)
                UI_PrintStringSmallNormal(">", 122u, 0u, 5u);
        }
    }

    ST7565_BlitFullScreen();
}

#ifndef ENABLE_CUSTOM_MENU_LAYOUT
static void UI_MENU_DrawTopRightRoundedBadge(const char *text, const uint8_t line, const bool center_in_area, const uint8_t area_x1, const uint8_t area_x2)
{
    const size_t length = strlen(text);
    const size_t char_pitch = ARRAY_SIZE(gFontSmall[0]) + 1u;
    const size_t text_width = length * char_pitch;
    const size_t capsule_span = text_width + 1u; // matches UI_PrintStringSmallNormalInverse x_end computation
    uint8_t text_x;

    if (length == 0 || line == 0 || line >= FRAME_LINES) {
        return;
    }

    if (center_in_area && area_x2 > area_x1 + 2u) {
        const uint8_t min_x = area_x1 + 1u;
        uint8_t max_x;
        const uint8_t area_width = area_x2 - area_x1 + 1u;

        if (capsule_span >= area_width) {
            text_x = min_x;
        } else {
            text_x = (uint8_t)(area_x1 + ((area_width - capsule_span) / 2u));
        }

        if (area_x2 > capsule_span) {
            max_x = (uint8_t)(area_x2 - capsule_span);
        } else {
            max_x = min_x;
        }

        if (max_x < min_x) {
            max_x = min_x;
        }
        if (text_x < min_x) {
            text_x = min_x;
        } else if (text_x > max_x) {
            text_x = max_x;
        }
    } else {
        if (capsule_span >= (LCD_WIDTH - 3u)) {
            text_x = 1u;
        } else {
            const uint8_t global_shift_right = 1u;
            const uint8_t base_text_x = (uint8_t)(LCD_WIDTH - capsule_span - 3u);
            const uint8_t max_text_x  = (uint8_t)(LCD_WIDTH - capsule_span - 1u);
            const uint16_t shifted_x = (uint16_t)base_text_x + global_shift_right;

            if (shifted_x > max_text_x) {
                text_x = max_text_x;
            } else {
                text_x = (uint8_t)shifted_x;
            }
        }
    }

    UI_PrintStringSmallNormalInverse(text, text_x, 0, line);
}
#endif /* ENABLE_CUSTOM_MENU_LAYOUT */

// ── Category selection screen ──────────────────────────────────────────────
// Renders one category per display-page (row).  ASCII-only because the CJK
// glyph height (12 px) does not fit in a single 8-px page row.
// Draw an 8×8 icon vertically centred inside a 16-pixel (2-page) LCD slot.
//
//  Vertical alignment (bit 0 = topmost pixel of each frame-buffer page):
//    CJK glyphs (12 px) occupy slot rows 0–11 → visual centre at row 5.5
//    8 px icon with 2 px top pad occupies slot rows 2–9 → centre at row 5.5
//    The 2 px shift is encoded as:
//      icon rows 0–5  →  page0 bits 2–7   (low 6 bits shifted left by 2)
//      icon rows 6–7  →  page1 bits 0–1   (high 2 bits shifted right by 6)
//
//  Parameters:
//    icon  8-byte column-major bitmap; byte[c] bit 0 = top, bit 7 = bottom.
//    x     starting LCD column; guard prevents gFrameBuffer OOB access.
//    page  top page of the 2-page slot (must be < FRAME_LINES - 1).
static void s_draw_icon_16px(const uint8_t *icon, uint8_t x, uint8_t page)
{
    for (uint8_t c = 0u; c < 8u && (x + c) < LCD_WIDTH; c++) {
        const uint8_t b = icon[c];
        /* icon rows 0–5 → frame page rows 2–7 (2 px top padding) */
        gFrameBuffer[page    ][x + c] |= (uint8_t)((b & 0x3Fu) << 2u);
        /* icon rows 6–7 → frame page+1 rows 0–1 */
        gFrameBuffer[page + 1u][x + c] |= (uint8_t)(b >> 6u);
    }
}

static void UI_DisplayMenuCat(void)
{
    static const char * const kCatNameEn[MENU_CAT_COUNT] = {
        "Channel", "Ch Mgmt", "Radio", "DTMF", "UI/Keys", "System"
    };
    static const char * const kCatNameZh[MENU_CAT_COUNT] = {
        "当前信道", "信道管理", "收发设置", "DTMF", "界面按键", "系统设置"
    };

    // 8×8 category icons, column-major (bit 0 = top, bit 7 = bottom).
    // Indexed by MENU_CAT_* (0 = VFO … 5 = System).
    //
    // VFO      : radio wave / frequency symbol — vertical bar (col 3) with
    //            two symmetric arcs on each side (cols 1,5 and 0,6)
    // ChanMgr  : stack of four horizontal bars (rows 1,3,5,7) — "list" icon
    // Radio    : classic walkie-talkie outline (antenna top, body rectangle,
    //            PTT side button)
    // DTMF     : 3×3 keypad dot grid (2×2 pixel dots at rows 1-2, 4-5, 7)
    // UI/Keys  : monitor outline (rows 1-5) + stand legs + base (rows 6-7)
    // System   : simplified 6-tooth gear outline
    static const uint8_t kCatIcons[MENU_CAT_COUNT][8] = {
        {0x08, 0x1C, 0x36, 0x63, 0x36, 0x1C, 0x08, 0x00}, // VFO      (radio waves)
        {0x00, 0x7E, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x7E}, // ChanMgr  (list bars)
        {0x10, 0x10, 0x7C, 0x44, 0x7C, 0x44, 0x7C, 0x00}, // Radio    (walkie-talkie)
        {0x00, 0x36, 0x00, 0xB6, 0x00, 0x36, 0x00, 0x00}, // DTMF     (keypad dots)
        {0x00, 0x3E, 0xA2, 0xE2, 0xE2, 0xA2, 0x3E, 0x00}, // UI/Keys  (monitor)
        {0x1C, 0x3E, 0x22, 0x77, 0x22, 0x3E, 0x1C, 0x00}, // System   (gear)
    };

    // 3 big-font items (16 px each = 2 pages) on pages 0–5; page 6 = indicator.
    const uint8_t visible = 3u;

    // Page-snap scrolling: always show a full page of 'visible' items.
    // With MENU_CAT_COUNT=6 and visible=3 there are exactly 2 pages:
    //   Page 0 (scroll=0): categories 0, 1, 2
    //   Page 1 (scroll=3): categories 3, 4, 5
    // Cursor moving from item 2 to item 3 flips the whole page at once.
    const uint8_t cur_page = (uint8_t)(gMenuCatCursor / visible);
    const uint8_t scroll   = (uint8_t)(cur_page * visible);

    // Selection indicator: left-pointing chevron (<) as 8×8 bitmap so that
    // s_draw_icon_16px() centres it vertically (2 px top pad, same as the
    // category icons).  Column-major, bit 0 = top pixel.
    //   col 0 (0x08): only row 3  → pointy tip
    //   col 3 (0x7F): rows 0–6   → flat back
    static const uint8_t kCatArrow[8] = {
        0x08, 0x1C, 0x3E, 0x7F, 0x00, 0x00, 0x00, 0x00,
    };

    UI_DisplayClear();

    for (uint8_t vi = 0u; vi < visible; vi++) {
        const uint8_t c    = scroll + vi;
        if (c >= MENU_CAT_COUNT) break;
        const uint8_t page = (uint8_t)(vi * 2u);

        // Icon: 8×8, vertically centred in the 16 px slot (2 px top / 6 px bottom).
        s_draw_icon_16px(kCatIcons[c], 1u, page);

        // Name: x = 13  (1 px left margin + 8 px icon + 4 px gap).
        const char *name = (g_lang == LANG_ZH) ? kCatNameZh[c] : kCatNameEn[c];
        if (g_lang == LANG_ZH)
            UI_PrintStringUTF8(name, 13, page);
        else
            UI_PrintString(name, 13, 0, page, 8);

        // Selection indicator: < chevron at right edge, vertically centred.
        if (c == gMenuCatCursor)
            s_draw_icon_16px(kCatArrow, (uint8_t)(LCD_WIDTH - 8u), page);
    }

    // Page indicator: "1/2" / "2/2" centred in the bottom row (page 6).
    // Small font char width = 6 px + 1 px spacing = 7 px; "1/2" = 3 chars = 21 px.
    {
        const uint8_t page_count = (MENU_CAT_COUNT + visible - 1u) / visible; // 2
        char page_str[8];
        // snprintf not available on bare-metal; build the string manually.
        page_str[0] = (char)('1' + cur_page);
        page_str[1] = '/';
        page_str[2] = (char)('0' + page_count);
        page_str[3] = '\0';
        // Centre the 3-char string (3 × 7 px = 21 px) on a 128 px screen.
        const uint8_t px = (uint8_t)((LCD_WIDTH - 21u) / 2u);
        UI_PrintStringSmallNormal(page_str, px, 0, 6);
    }
}

void UI_DisplayMenu(void)
{
    // Category top-level screen: show before any item rendering
    if (USE_CAT_MENU() && gMenuCategory == MENU_CAT_NONE) {
        UI_DisplayMenuCat();
        ST7565_BlitFullScreen();
        return;
    }

    // Dedicated full-screen editor for channel name
    if (UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME && gIsInSubMenu && edit_index >= 0) {
        UI_DisplayNameEdit();
        return;
    }

    const unsigned int menu_list_width = 6; // kept for fallback layout (ENABLE_CUSTOM_MENU_LAYOUT off)
    const unsigned int menu_item_x1    = 0;
    const unsigned int menu_item_x2    = LCD_WIDTH - 1;
    unsigned int       i;
    char               String[64];  // bigger cuz we can now do multi-line in one string (use '\n' char)
    char               top_right_badge[16];

    const int m = UI_MENU_GetCurrentMenuId();

#ifdef ENABLE_DTMF_CALLING
    char               Contact[16];
#endif

    UI_DisplayClear();

#ifndef ENABLE_CUSTOM_MENU_LAYOUT
        // original (fallback) menu layout — sidebar list + right value zone
    {
        // Use a category-local cursor for bounds checks so we never render
        // items from adjacent categories or hidden entries past the end.
        const int local_cursor = (USE_CAT_MENU() && gMenuCategory < MENU_CAT_COUNT)
            ? (int)(gMenuCursor - gMenuCatFirstIdx[gMenuCategory])
            : (int)gMenuCursor;
        for (i = 0; i < 3; i++)
            if (local_cursor > 0 || i > 0)
                if ((gMenuListCount - 1) != local_cursor || i != 2)
                    UI_PrintString(MenuList[gMenuCursor + i - 1].name, 0, 0, i * 2, 8);
    }

    // invert the current menu list item pixels
    for (i = 0; i < (8 * menu_list_width); i++)
    {
        gFrameBuffer[2][i] ^= 0xFF;
        gFrameBuffer[3][i] ^= 0xFF;
    }

    // draw vertical separating dotted line
    for (i = 0; i < 7; i++)
        gFrameBuffer[i][(8 * menu_list_width) + 1] = 0xAA;

    // draw the little sub-menu triangle marker
    if (gIsInSubMenu)
        memcpy(gFrameBuffer[0] + (8 * menu_list_width) + 1, BITMAP_CurrentIndicator, sizeof(BITMAP_CurrentIndicator));

    // draw the menu index number/count
    sprintf(String, "%2u.%u",
        1u + (USE_CAT_MENU() && gMenuCategory < MENU_CAT_COUNT
              ? (unsigned)(gMenuCursor - gMenuCatFirstIdx[gMenuCategory])
              : (unsigned)gMenuCursor),
        gMenuListCount);

    UI_PrintStringSmallNormal(String, 2, 0, 6);

#else
    {   // haige-style full-width layout:
        //   pages 0-1 = 16px title bar  (name left | N/Total counter right | separator bottom)
        //   pages 2-7 = 48px value area (content vertically centred)
        const int menu_index = gMenuCursor;

        // Counter "N/Total": render small font at page 0 then bit-shift 4px downward so it is
        // vertically centred in the 16px title bar.  ST7565 column bytes are LSB-first (bit0=top):
        //   b << 4  moves character pixels to rows 4-7 of page 0
        //   b >> 4  places the remainder  into rows 0-3 of page 1
        sprintf(String, "%u/%u",
            1u + (USE_CAT_MENU() && gMenuCategory < MENU_CAT_COUNT
                  ? (unsigned)(gMenuCursor - gMenuCatFirstIdx[gMenuCategory])
                  : (unsigned)gMenuCursor),
            gMenuListCount);
        {
            const uint8_t cnt_len = (uint8_t)strlen(String);
            const uint8_t cnt_x   = (uint8_t)(LCD_WIDTH - (unsigned)cnt_len * 7u);
            UI_PrintStringSmallNormal(String, cnt_x, 0, 0);
            for (uint8_t col = cnt_x; col < LCD_WIDTH; col++) {
                const uint8_t b = gFrameBuffer[0][col];
                if (b) {
                    gFrameBuffer[0][col] = (uint8_t)(b << 4);
                    gFrameBuffer[1][col] |= (uint8_t)(b >> 4);
                }
            }
        }

        // Title: current item name, big font (16px, fills pages 0-1), left-aligned.
        // Big font writes only the LEFT columns of pages 0-1; the counter occupies the
        // RIGHT columns, so there is no conflict as long as the title is ≤ ~12 chars.
        if (menu_index >= 0 && menu_index < (int)ARRAY_SIZE(MenuList) && MenuList[menu_index].name[0] != '\0') {
            const char *_mn = MENU_ItemName(&MenuList[menu_index]);
            if (g_lang != LANG_EN)
                UI_PrintStringUTF8(_mn, 0, 0);
            else
                UI_PrintString(_mn, 0, 0, 0, 8);
        }

        // Dotted horizontal separator: set the bottom pixel of every column in page 1
        for (i = 0; i < LCD_WIDTH; i++)
            gFrameBuffer[1][i] |= 0x80;
    }
#endif

    // **************

    memset(String, 0, sizeof(String));
    memset(top_right_badge, 0, sizeof(top_right_badge));

    bool already_printed = false;

    /* Brightness is set to max in some entries of this menu. Return it to the configured brightness
       level the "next" time we enter here.I.e., when we move from one menu to another.
       It also has to be set back to max when pressing the Exit key. */

    BACKLIGHT_TurnOn();

    //#if !defined(ENABLE_SPECTRUM) || !defined(ENABLE_FMRADIO)
        uint8_t gaugeLine = 0;
        uint8_t gaugeMin = 0;
        uint8_t gaugeMax = 0;
    //#endif

    switch (m)
    {
        case MENU_SQL:
            sprintf(String, "%d", gSubMenuSelection);
            break;

        case MENU_MIC:
            {   // display the mic gain in actual dB rather than just an index number
                const uint8_t mic = gMicGain_dB2[gSubMenuSelection];
                sprintf(String, "+%u.%udB", mic / 2, (mic % 2) * 5);

                gaugeLine = 6;
                gaugeMin = 0;
                gaugeMax = 8;
            }
            break;

        case MENU_MIC_BAR:
            #ifdef ENABLE_AUDIO_BAR
                strcpy(String, SUBV(gSubMenu_OFF_ON[gSubMenuSelection], gSubMenu_OFF_ON_zh[gSubMenuSelection]));
            #else
                strcpy(String, gSubMenu_NA);
            #endif
            break;

        case MENU_STEP: {
            uint16_t step = gStepFrequencyTable[FREQUENCY_GetStepIdxFromSortedIdx(gSubMenuSelection)];
            sprintf(String, "%d.%02ukHz", step / 100, step % 100);
            break;
        }

        case MENU_TXP:
        {
            uint8_t sel = (uint8_t)gSubMenuSelection < 4u ? (uint8_t)gSubMenuSelection : 0u;
            strcpy(String, SUBV(gSubMenu_TXP[sel], gSubMenu_TXP_zh[sel]));
            break;
        }

        case MENU_R_DCS:
        case MENU_T_DCS:
            if (gSubMenuSelection == 0)
                strcpy(String, SUBV(gSubMenu_OFF_ON[0], gSubMenu_OFF_ON_zh[0]));
            else if (gSubMenuSelection < 105)
                sprintf(String, "D%03oN", DCS_Options[gSubMenuSelection -   1]);
            else
                sprintf(String, "D%03oI", DCS_Options[gSubMenuSelection - 105]);
            break;

        case MENU_R_CTCS:
        case MENU_T_CTCS:
        {
            if (gSubMenuSelection == 0)
                strcpy(String, SUBV(gSubMenu_OFF_ON[0], gSubMenu_OFF_ON_zh[0]));
            else
                sprintf(String, "%u.%uHz", CTCSS_Options[gSubMenuSelection - 1] / 10, CTCSS_Options[gSubMenuSelection - 1] % 10);
            break;
        }

        case MENU_SFT_D:
            strcpy(String, SUBV(gSubMenu_SFT_D[gSubMenuSelection], gSubMenu_SFT_D_zh[gSubMenuSelection]));
            break;

        case MENU_OFFSET:
            if (!gIsInSubMenu || gInputBoxIndex == 0)
            {
                sprintf(String, "%d.%05u MHz", gSubMenuSelection / 100000, abs(gSubMenuSelection) % 100000);
            }
            else
            {
                const char * ascii = INPUTBOX_GetAscii();
                sprintf(String, "%.3s.%.3s MHz", ascii, ascii + 3);
            }
            UI_PrintString(String, menu_item_x1, menu_item_x2, 4, 8);
            already_printed = true;
            break;

        case MENU_W_N:
            strcpy(String, SUBV(gSubMenu_W_N[gSubMenuSelection], gSubMenu_W_N_zh[gSubMenuSelection]));
            break;

#ifndef ENABLE_FEAT_F4HWN
        case MENU_SCR:
            strcpy(String, SUBV(gSubMenu_SCRAMBLER[gSubMenuSelection], gSubMenu_SCRAMBLER_zh[gSubMenuSelection]));
            #if 1
                if (gSubMenuSelection > 0 && gSetting_ScrambleEnable)
                    BK4819_EnableScramble(gSubMenuSelection - 1);
                else
                    BK4819_DisableScramble();
            #endif
            break;
#endif

        case MENU_VOX:
            #ifdef ENABLE_VOX
                if (gSubMenuSelection == 0)
                    strcpy(String, SUBV(gSubMenu_OFF_ON[0], gSubMenu_OFF_ON_zh[0]));
                else
                    sprintf(String, "%u", gSubMenuSelection);
            #else
                strcpy(String, gSubMenu_NA);
            #endif
            break;

        case MENU_ABR:
            if(gSubMenuSelection == 0)
            {
                strcpy(String, SUBV(gSubMenu_OFF_ON[0], gSubMenu_OFF_ON_zh[0]));
            }
            else if(gSubMenuSelection < 7)
            {
                sprintf(String, "%ds", gSubMenuSelection * 5);
                gaugeLine = 6;
                gaugeMin = 1;
                gaugeMax = 6;
            }
            else
            {
                strcpy(String, SUBV("ON", "\xe5\xb8\xb8\xe4\xba\xae")); /* 常亮 */
            }

            // Obsolete ???
            //if(BACKLIGHT_GetBrightness() < 4)
            //    BACKLIGHT_SetBrightness(4);
            break;

        case MENU_ABR_MIN:
        case MENU_ABR_MAX:
            sprintf(String, "%d", gSubMenuSelection);
            if(gIsInSubMenu)
                BACKLIGHT_SetBrightness(gSubMenuSelection);
            // Obsolete ???
            //else if(BACKLIGHT_GetBrightness() < 4)
            //    BACKLIGHT_SetBrightness(4);
            break;

        case MENU_AM:
            strcpy(String, gModulationStr[gSubMenuSelection]);
            break;

        case MENU_AUTOLK:
            if (gSubMenuSelection == 0)
                strcpy(String, SUBV(gSubMenu_OFF_ON[0], gSubMenu_OFF_ON_zh[0]));
            else
            {
                sprintf(String, "%02dm:%02ds", ((gSubMenuSelection * 15) / 60), ((gSubMenuSelection * 15) % 60));
                gaugeLine = 6;
                gaugeMin = 1;
                gaugeMax = 40;
            }
            break;

        case MENU_COMPAND:
        case MENU_ABR_ON_TX_RX:
            strcpy(String, SUBV(gSubMenu_RX_TX[gSubMenuSelection], gSubMenu_RX_TX_zh[gSubMenuSelection]));
            break;

        #ifndef ENABLE_FEAT_F4HWN
            #ifdef ENABLE_AM_FIX
                case MENU_AM_FIX:
            #endif
        #endif
        case MENU_BCL:
        case MENU_BEEP:
        case MENU_STE:
        case MENU_D_ST:
#ifdef ENABLE_DTMF_CALLING
        case MENU_D_DCD:
#endif
        case MENU_D_LIVE_DEC:
        #ifdef ENABLE_NOAA
            case MENU_NOAA_S:
        #endif
#ifndef ENABLE_FEAT_F4HWN
        case MENU_350TX:
        case MENU_200TX:
        case MENU_500TX:
#endif
        case MENU_350EN:
#ifndef ENABLE_FEAT_F4HWN
        case MENU_SCREN:
#endif
#ifdef ENABLE_FEAT_F4HWN
        case MENU_SET_TMR:
        case MENU_S_PRI:
#endif
            strcpy(String, SUBV(gSubMenu_OFF_ON[gSubMenuSelection], gSubMenu_OFF_ON_zh[gSubMenuSelection]));
            break;

        case MENU_MEM_CH:
        case MENU_1_CALL:
        case MENU_DEL_CH:
        case MENU_S_PRI_CH_1:
        case MENU_S_PRI_CH_2:
        {
            if(gSubMenuSelection > MR_CHANNEL_LAST)  // covers 0xFFFF (uninit EEPROM) and "None" sentinel
            {
                print_menu_value(SUBV("None", "\xe6\x97\xa0"), menu_item_x1, menu_item_x2, 4);
                already_printed = true;
                break;
            }
            else
            {
                const bool valid = RADIO_CheckValidChannel(gSubMenuSelection, false, 0);

                UI_GenerateChannelStringEx(String, valid, gSubMenuSelection);
                UI_PrintString(String, menu_item_x1, menu_item_x2, 2, 8);

                if (valid && !gAskForConfirmation)
                {   // show the frequency so that the user knows the channels frequency
                    const uint32_t frequency = SETTINGS_FetchChannelFrequency(gSubMenuSelection);
                    sprintf(String, "%u.%05u", frequency / 100000, frequency % 100000);
                    UI_PrintStringSmallNormal(String, menu_item_x1, menu_item_x2, 6); // line 6 = gFrameBuffer[6], last page
                }

                SETTINGS_FetchChannelName(String, gSubMenuSelection);
                print_menu_value(String[0] ? String : "--", menu_item_x1, menu_item_x2, 4);
                already_printed = true;
                break;
            }
        }

        case MENU_MEM_NAME:
        {
            const bool valid = RADIO_CheckValidChannel(gSubMenuSelection, false, 0);

            UI_GenerateChannelStringEx(String, valid, gSubMenuSelection);
            UI_PrintString(String, menu_item_x1, menu_item_x2, 2, 8);

            if (valid)
            {
                const uint32_t frequency = SETTINGS_FetchChannelFrequency(gSubMenuSelection);

                if (!gIsInSubMenu)
                    edit_index = -1;
                if (edit_index < 0)
                {   // show the channel name (not editing)
                    SETTINGS_FetchChannelName(String, gSubMenuSelection);
                    char *pPrintStr = String[0] ? String : "--";
                    print_menu_value(pPrintStr, menu_item_x1, menu_item_x2, 4);
                }

                if (!gAskForConfirmation)
                {
                    sprintf(String, "%u.%05u", frequency / 100000, frequency % 100000);
                    UI_PrintStringSmallNormal(String, menu_item_x1, menu_item_x2, 6);
                }
            }

            already_printed = true;
            break;
        }

        case MENU_SAVE:
            if (gSubMenuSelection == 0)
                strcpy(String, SUBV(gSubMenu_OFF_ON[0], gSubMenu_OFF_ON_zh[0]));
            else
                sprintf(String, "1:%u", gSubMenuSelection);
            break;

        case MENU_TDR:
            strcpy(String, SUBV(gSubMenu_RXMode[gSubMenuSelection], gSubMenu_RXMode_zh[gSubMenuSelection]));
            break;

        case MENU_TOT:
        {
            const uint16_t secs = gSubMenuSelection * 30;
            sprintf(String, "%02dm:%02ds", secs / 60, secs % 60);
            gaugeLine = 6;
            gaugeMin = 1;
            gaugeMax = 30;
            break;
        }

        #ifdef ENABLE_VOICE
            case MENU_VOICE:
                strcpy(String, SUBV(gSubMenu_VOICE[gSubMenuSelection], gSubMenu_VOICE_zh[gSubMenuSelection]));
                break;
        #endif

        case MENU_SC_REV:
            if(gSubMenuSelection == 0)
            {
                strcpy(String, SUBV("STOP", "\xe5\x81\x9c\xe6\xad\xa2")); /* 停止 */
            }
            else if(gSubMenuSelection < 81)
            {
                const char *fmt = SUBV("CARRIER\n%02ds:%03dms", "\xe8\xbd\xbd\xe6\xb3\xa2\n%02ds:%03dms"); /* 载波 */
                sprintf(String, fmt, ((gSubMenuSelection * 250) / 1000), ((gSubMenuSelection * 250) % 1000));
                gaugeLine = 6;
                gaugeMin = 1;
                gaugeMax = 80;
            }
            else
            {
                const char *fmt = SUBV("TIMEOUT\n%02dm:%02ds", "\xe8\xb6\x85\xe6\x97\xb6\n%02dm:%02ds"); /* 超时 */
                sprintf(String, fmt, (((gSubMenuSelection - 80) * 5) / 60), (((gSubMenuSelection - 80) * 5) % 60));
                gaugeLine = 6;
                gaugeMin = 80;
                gaugeMax = 104;
            }
            break;

        case MENU_MDF:
            strcpy(String, SUBV(gSubMenu_MDF[gSubMenuSelection], gSubMenu_MDF_zh[gSubMenuSelection]));
            break;

        case MENU_RP_STE:
            if (gSubMenuSelection == 0)
                strcpy(String, SUBV(gSubMenu_OFF_ON[0], gSubMenu_OFF_ON_zh[0]));
            else
                sprintf(String, "%u*100ms", gSubMenuSelection);
            break;

        case MENU_LIST_CH:
        case MENU_S_LIST:
            if (gSubMenuSelection == MR_CHANNELS_LIST + 1)
                strcpy(String, SUBV("ALL", "\xe5\x85\xa8\xe9\x83\xa8")); /* 全部 */
            else if (gSubMenuSelection == 0 && m == MENU_LIST_CH)
                strcpy(String, SUBV("OFF", "\xe5\x85\xb3")); /* 关 */
            else {
                const char *name = gListName[gSubMenuSelection - 1];
                
                // If first character is empty/invalid, display "N/A"
                if (IsEmptyName(name, sizeof(gListName[0])))
                    sprintf(String, "%02u", gSubMenuSelection);
                else
                    sprintf(String, "%02u (%.3s)", gSubMenuSelection, name);
            }
            break;
            
        #ifdef ENABLE_ALARM
            case MENU_AL_MOD:
                sprintf(String, "%s", SUBV(gSubMenu_AL_MOD[gSubMenuSelection], gSubMenu_AL_MOD_zh[gSubMenuSelection]));
                break;
        #endif

#ifdef ENABLE_DTMF_CALLING
        case MENU_ANI_ID:
            strcpy(String, gEeprom.ANI_DTMF_ID);
            break;
#endif
        case MENU_UPCODE:
            sprintf(String, "%.8s\n%.8s", gEeprom.DTMF_UP_CODE, gEeprom.DTMF_UP_CODE + 8);
            break;

        case MENU_DWCODE:
            sprintf(String, "%.8s\n%.8s", gEeprom.DTMF_DOWN_CODE, gEeprom.DTMF_DOWN_CODE + 8);
            break;

#ifdef ENABLE_DTMF_CALLING
        case MENU_D_RSP:
            strcpy(String, SUBV(gSubMenu_D_RSP[gSubMenuSelection], gSubMenu_D_RSP_zh[gSubMenuSelection]));
            break;

        case MENU_D_HOLD:
            sprintf(String, "%ds", gSubMenuSelection);
            break;
#endif
        case MENU_D_PRE:
            sprintf(String, "%d*10ms", gSubMenuSelection);
            break;

        case MENU_PTT_ID:
            strcpy(String, SUBV(gSubMenu_PTT_ID[gSubMenuSelection], gSubMenu_PTT_ID_zh[gSubMenuSelection]));
            break;

        case MENU_BAT_TXT:
            strcpy(String, SUBV(gSubMenu_BAT_TXT[gSubMenuSelection], gSubMenu_BAT_TXT_zh[gSubMenuSelection]));
            break;

#ifdef ENABLE_DTMF_CALLING
        case MENU_D_LIST:
            gIsDtmfContactValid = DTMF_GetContact((int)gSubMenuSelection - 1, Contact);
            if (!gIsDtmfContactValid)
                strcpy(String, "NULL");
            else
                memcpy(String, Contact, 8);
            break;
#endif

        case MENU_PONMSG:
            strcpy(String, SUBV(gSubMenu_PONMSG[gSubMenuSelection], gSubMenu_PONMSG_zh[gSubMenuSelection]));
            break;

        case MENU_ROGER:
            strcpy(String, SUBV(gSubMenu_ROGER[gSubMenuSelection], gSubMenu_ROGER_zh[gSubMenuSelection]));
            break;

        case MENU_VOL: {
            // SysInf is paginated. Pages appear in this order, only when their
            // feature flag is enabled:
            //   0          -> identity
            //   next       -> Build date/time         (ENABLE_FEAT_F4HWN)
            //   next       -> Battery                 (ENABLE_FEAT_F4HWN)
            //   next       -> Flash / SRAM usage      (ENABLE_FEAT_F4HWN_MEM)
            //   next, +1   -> CODE / WIKI QR codes    (ENABLE_FEAT_F4HWN_QRCODE)
            // In non-F4HWN builds, page 0 keeps the old battery-voltage display.
            const uint8_t page = (uint8_t)gSubMenuSelection;
            uint8_t       p    = 0;

            if (page == p++) {
                // Page 0: firmware identity — draw all three items manually so
                // Edition (page 6) is not overwritten by the generic text loop.
                print_menu_value(AUTHOR_STRING, menu_item_x1, menu_item_x2, 2);
                print_menu_value(VERSION_STRING, menu_item_x1, menu_item_x2, 4);
                UI_PrintStringSmallNormal(Edition, menu_item_x1, menu_item_x2, 6);
                already_printed = true;
                break;
            }

            if (page == p++) {
                char val[24];

                sprintf(val, "%u.%02uV  %u%%",
                    gBatteryVoltageAverage / 100, gBatteryVoltageAverage % 100,
                    BATTERY_VoltsToPercent(gBatteryVoltageAverage));
                UI_PrintStringSmallNormal(val, 0, menu_item_x2, 3);

                UI_PrintStringSmallNormal(gSubMenu_BATTYP[gEeprom.BATTERY_TYPE], 0, menu_item_x2, 5);

                already_printed = true;
                break;
            }
#ifdef ENABLE_FEAT_F4HWN_MEM
            if (page == p++) {
                uint16_t flash_pct = 0;
                uint16_t ram_pct   = 0;
                UI_GetMemPercents(&flash_pct, &ram_pct);

                char val[16];

                // MEMORY title capsule centered in right zone, fb line 1.
                strcpy(top_right_badge, "MEMORY");

                // Flash + SRAM values stacked below, normal small font, with a fb-line of breathing space.
                sprintf(val, "FLASH %u.%u%%",
                        (unsigned)(flash_pct / 100), (unsigned)((flash_pct / 10) % 10));
                UI_PrintStringSmallNormal(val, 0, menu_item_x2, 3);

                sprintf(val, "SRAM  %u.%u%%",
                        (unsigned)(ram_pct / 100), (unsigned)((ram_pct / 10) % 10));
                UI_PrintStringSmallNormal(val, 0, menu_item_x2, 5);

                already_printed = true;
                break;
            }
#endif
#ifdef ENABLE_FEAT_F4HWN_QRCODE
            // Right zone: x=49..127 (79 px). QR centered at x=72..104.
            // Capsule label above QR (small-font Inverse style at fb line 1).
            if (page == p || page == p + 1) {
                const bool is_wiki = (page == (p + 1));

                strcpy(top_right_badge, is_wiki ? "WIKI" : "CODE");
                UI_DrawQRCode(is_wiki, 72, 28);
                
                already_printed = true;
                break;
            }

            p += 2; 
#endif
            break;
        }

        case MENU_RESET:
            strcpy(String, SUBV(gSubMenu_RESET[gSubMenuSelection], gSubMenu_RESET_zh[gSubMenuSelection]));
            break;

        case MENU_F_LOCK:
#ifdef ENABLE_FEAT_F4HWN
            if(!gIsInSubMenu && gUnlockAllTxConfCnt>0 && gUnlockAllTxConfCnt<3)
#else
            if(!gIsInSubMenu && gUnlockAllTxConfCnt>0 && gUnlockAllTxConfCnt<10)
#endif
                strcpy(String, SUBV("READ\nMANUAL", "\xe9\x98\x85\xe8\xaf\xbb\n\xe6\x89\x8b\xe5\x86\x8c")); /* 阅读手册 */
            else
                strcpy(String, SUBV(gSubMenu_F_LOCK[gSubMenuSelection], gSubMenu_F_LOCK_zh[gSubMenuSelection]));
            break;

        #ifdef ENABLE_F_CAL_MENU
            case MENU_F_CALI:
                {
                    const uint32_t value   = 22656 + gSubMenuSelection;
                    const uint32_t xtal_Hz = (0x4f0000u + value) * 5;

                    writeXtalFreqCal(gSubMenuSelection, false);

                    sprintf(String, "%d\n%u.%06u\nMHz",
                        gSubMenuSelection,
                        xtal_Hz / 1000000, xtal_Hz % 1000000);
                }
                break;
        #endif

        case MENU_BATCAL:
        {
            const uint16_t vol = (uint32_t)gBatteryVoltageAverage * gBatteryCalibration[3] / gSubMenuSelection;
            sprintf(String, "%u.%02uV\n%u", vol / 100, vol % 100, gSubMenuSelection);
            break;
        }

        case MENU_BATTYP:
            strcpy(String, gSubMenu_BATTYP[gSubMenuSelection]);
            break;

        case MENU_LANG:
            strcpy(String, gSubMenu_LANG[gSubMenuSelection]);
            break;

        case MENU_SET_NAV:
            strcpy(String, SUBV(gSubMenu_SET_NAV[gSubMenuSelection], gSubMenu_SET_NAV_zh[gSubMenuSelection]));
            break;

        case MENU_F1SHRT:
        case MENU_F1LONG:
        case MENU_F2SHRT:
        case MENU_F2LONG:
        case MENU_MLONG:
            strcpy(String, SUBV(gSubMenu_SIDEFUNCTIONS[gSubMenuSelection].name,
                                gSubMenu_SIDEFUNCTIONS_zh[gSubMenuSelection]));
            break;

#ifdef ENABLE_FEAT_F4HWN_SLEEP
        case MENU_SET_OFF:
            if(gSubMenuSelection == 0)
            {
                strcpy(String, SUBV(gSubMenu_OFF_ON[0], gSubMenu_OFF_ON_zh[0]));
            }
            else
            {
                const uint8_t mins = gSubMenuSelection * 5;
                sprintf(String, "%dh:%02dm", mins / 60, mins % 60);
                gaugeLine = 6;
                gaugeMin = 1;
                gaugeMax = 24;
            }
            break;
#endif

#ifdef ENABLE_FEAT_F4HWN
        case MENU_SET_PTT:
            strcpy(String, SUBV(gSubMenu_SET_PTT[gSubMenuSelection], gSubMenu_SET_PTT_zh[gSubMenuSelection]));
            break;

        case MENU_SET_TOT:
        case MENU_SET_EOT:
            strcpy(String, SUBV(gSubMenu_SET_TOT[gSubMenuSelection], gSubMenu_SET_TOT_zh[gSubMenuSelection]));
            break;

        case MENU_SET_CTR:
            #ifdef ENABLE_FEAT_F4HWN_CTR
                sprintf(String, "%d", gSubMenuSelection);
                gSetting_set_ctr = gSubMenuSelection;
                ST7565_ContrastAndInv();
            #else
                strcpy(String, gSubMenu_NA);
            #endif
            break;

        case MENU_TX_LOCK:
            if(TX_freq_check(gEeprom.VfoInfo[gEeprom.TX_VFO].pTX->Frequency) == 0)
            {
                strcpy(String, "IN PLAN"); /* chars 在/范/围 absent from CJK font */
            }
            else
            {
                strcpy(String, SUBV(gSubMenu_OFF_ON[gSubMenuSelection], gSubMenu_OFF_ON_zh[gSubMenuSelection]));
            }
            break;

        case MENU_SET_MET:
        case MENU_SET_GUI:
            strcpy(String, SUBV(gSubMenu_SET_MET[gSubMenuSelection], gSubMenu_SET_MET_zh[gSubMenuSelection]));
            break;

        #ifdef ENABLE_FEAT_F4HWN_SCAN_FASTER
            case MENU_SET_SCN:
                strcpy(String, SUBV(gSubMenu_SET_SCN[gSubMenuSelection], gSubMenu_SET_SCN_zh[gSubMenuSelection]));
                break;
        #endif /* ENABLE_FEAT_F4HWN_SCAN_FASTER */

        #ifdef ENABLE_FEAT_F4HWN_AUDIO
            case MENU_SET_AUD:
                if(gTxVfo->Modulation == MODULATION_AM) {
                    strcpy(String, SUBV(gSubMenu_SET_AUD_AM[gSubMenuSelection], gSubMenu_SET_AUD_AM_zh[gSubMenuSelection]));
                    strcpy(top_right_badge, "AM");
                }
                else if (gTxVfo->Modulation == MODULATION_USB) {
                    strcpy(String, "USB");
                    strcpy(top_right_badge, "USB");
                }
                else {
                    strcpy(String, SUBV(gSubMenu_SET_AUD_FM[gSubMenuSelection], gSubMenu_SET_AUD_FM_zh[gSubMenuSelection]));
                    strcpy(top_right_badge, "FM");
                }
                break;
        #endif /* ENABLE_FEAT_F4HWN_AUDIO */

        #ifdef ENABLE_FEAT_F4HWN_NARROWER
            case MENU_SET_NFM:
                strcpy(String, SUBV(gSubMenu_SET_NFM[gSubMenuSelection], gSubMenu_SET_NFM_zh[gSubMenuSelection]));
                break;
        #endif /* ENABLE_FEAT_F4HWN_NARROWER */

        #ifdef ENABLE_FEAT_F4HWN_VOL
            case MENU_SET_VOL:
                if(gSubMenuSelection == 0)
                {
                    strcpy(String, SUBV(gSubMenu_OFF_ON[0], gSubMenu_OFF_ON_zh[0]));
                }
                else if(gSubMenuSelection < 64)
                {
                    sprintf(String, "%02u", gSubMenuSelection);
                    //#if !defined(ENABLE_SPECTRUM) || !defined(ENABLE_FMRADIO)
                    //ST7565_Gauge(4, 1, 63, gSubMenuSelection);
                    gaugeLine = 6;
                    gaugeMin = 1;
                    gaugeMax = 63;
                    //#endif
                }
                // gEeprom.VOLUME_GAIN = gSubMenuSelection;
                BK4819_SetRxAudioGain();
                break;
        #endif

        #ifdef ENABLE_FEAT_F4HWN_RESCUE_OPS
            case MENU_SET_KEY:
                strcpy(String, gSubMenu_SET_KEY[gSubMenuSelection]);
                break;                
        #endif
#endif

    }

    //#if !defined(ENABLE_SPECTRUM) || !defined(ENABLE_FMRADIO)
    if(gaugeLine != 0)
    {
        ST7565_Gauge(gaugeLine, gaugeMin, gaugeMax, gSubMenuSelection);
    }
    //#endif

    if (!already_printed)
    {   // we now do multi-line text in a single string

        unsigned int y;
        unsigned int lines = 1;
        unsigned int len   = strlen(String);
        bool         small = false;

        if (len > 0)
        {
            // count number of lines
            for (i = 0; i < len; i++)
            {
                if (String[i] == '\n' && i < (len - 1))
                {   // found new line char
                    lines++;
                    String[i] = 0;  // null terminate the line
                }
            }

            if (lines > 3)
            {   // use small text
                small = true;
                if (lines > 7)
                    lines = 7;
            }

            // center vertically'ish
            /*
            if (small)
                y = 3 - ((lines + 0) / 2);  // untested
            else
                y = 2 - ((lines + 0) / 2);
            */

            // Centre content in the value area (pages 2-7, 48px).
            // big font : each line occupies 2 pages → y = 5 - lines (1 line→4, 2→3, 3→2)
            // small font: each line occupies 1 page → y = 2 + (6 - lines) / 2
            if (small)
                y = 2u + (6u - (lines < 6u ? lines : 6u)) / 2u;
            else
                y = (lines < 4u) ? (5u - lines) : 2u;

            // When a gauge bar occupies page 6, position text with one blank page of
            // breathing room below the title-bar separator.
            // 1-line big font: text at pages 3-4 (page 2 blank), gauge at page 6.
            // 2-line big font: text at pages 2-5 (flush to top), gauge at page 6
            //   (no extra space; pushing further would collide with the gauge).
            if (gaugeLine != 0) {
                if (!small && lines <= 1u)
                    y = 4u;  // same as standard 1-line center
                else
                    y = 2u;
            }

            // draw the text lines
            for (i = 0; i < len && lines > 0; lines--)
            {
                if (small)
                    UI_PrintStringSmallNormal(String + i, menu_item_x1, menu_item_x2, y);
                else
                    print_menu_value(String + i, menu_item_x1, menu_item_x2, y);

                // look for start of next line
                while (i < len && String[i] >= 32)
                    i++;

                // hop over the null term char(s)
                while (i < len && String[i] < 32)
                    i++;

                y += small ? 1 : 2;
            }
        }
    }

    if (m == MENU_S_PRI_CH_1 || m == MENU_S_PRI_CH_2)
    {

    }

    if ((m == MENU_R_CTCS || m == MENU_R_DCS) && gCssBackgroundScan)
        UI_PrintString("SCAN", menu_item_x1, menu_item_x2, 4, 8);

#ifdef ENABLE_DTMF_CALLING
    if (m == MENU_D_LIST && gIsDtmfContactValid) {
        Contact[11] = 0;
        memcpy(&gDTMF_ID, Contact + 8, 4);
        sprintf(String, "ID:%4s", gDTMF_ID);
        UI_PrintString(String, menu_item_x1, menu_item_x2, 4, 8);
    }
#endif

    if (m == MENU_R_CTCS || m == MENU_T_CTCS)
        sprintf(top_right_badge, "%u/%u", (unsigned)gSubMenuSelection, (unsigned)ARRAY_SIZE(CTCSS_Options));

    if (m == MENU_R_DCS || m == MENU_T_DCS)
        sprintf(top_right_badge, "%u/%u", (unsigned)gSubMenuSelection, (unsigned)ARRAY_SIZE(DCS_Options) * 2u);

#ifdef ENABLE_DTMF_CALLING
    if (m == MENU_D_LIST) {
        sprintf(top_right_badge, "%03d", gSubMenuSelection);
    }
#endif

    if (top_right_badge[0] != '\0') {
#ifdef ENABLE_CUSTOM_MENU_LAYOUT
        // Left-aligned, no inverted background; 2px gap below separator line
        UI_PrintStringSmallNormal(top_right_badge, 0, 0, 2);
        {
            const uint8_t idx_w = (uint8_t)(strlen(top_right_badge) * 7u);
            for (uint8_t col = 0; col < idx_w; col++) {
                const uint8_t b = gFrameBuffer[2][col];
                if (b) {
                    gFrameBuffer[2][col] = (uint8_t)(b << 2);
                    gFrameBuffer[3][col] |= (uint8_t)(b >> 6);
                }
            }
        }
#else
        UI_MENU_DrawTopRightRoundedBadge(top_right_badge, 2, false, 0, LCD_WIDTH - 1);
#endif
    }

    if ((m == MENU_RESET    ||
         m == MENU_MEM_CH   ||
         m == MENU_MEM_NAME ||
         m == MENU_DEL_CH) && gAskForConfirmation)
    {   // display confirmation
        char *pPrintStr = (gAskForConfirmation == 1) ? "SURE?" : "WAIT!";
        UI_PrintString(pPrintStr, menu_item_x1, menu_item_x2, 4, 8);
    }

#ifdef ENABLE_CUSTOM_MENU_LAYOUT
    // Edit mode indicator: '>' at far-right of value area (pages 2-3), shown only while in submenu
    if (gIsInSubMenu)
        UI_PrintString(">", LCD_WIDTH - 8, 0, 2, 8);
#endif

    ST7565_BlitFullScreen();
}
