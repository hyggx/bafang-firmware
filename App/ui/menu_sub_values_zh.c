/* SPDX-License-Identifier: Apache-2.0
 * Simplified Chinese strings for sub-menu option values.
 *
 * Each array is a parallel counterpart to the English gSubMenu_* arrays in
 * menu.c.  The #ifdef guards MUST mirror those that wrap the English arrays so
 * that every zh array has the same element count as its English twin.
 * The SUBV(en, zh) macro in menu.c selects the right array at runtime.
 *
 * All strings are UTF-8 encoded.  ASCII-only entries (numbers, product names,
 * technical codes) keep their English text unchanged.
 */

#include "menu_sub_values_zh.h"

/* ── OFF / ON ──────────────────────────────────────────────────────────── */
const char * const gSubMenu_OFF_ON_zh[] = {
    "\xe5\x85\xb3",   /* 关  OFF  */
    "\xe5\xbc\x80",   /* 开  ON   */
};

/* ── TX power level names ───────────────────────────────────────────────── */
const char * const gSubMenu_TXP_zh[] = {
    "\xe7\x94\xa8\xe6\x88\xb7",   /* 用户  USER   */
    "\xe4\xbd\x8e" "1",           /* 低1   LOW 1  */
    "\xe4\xbd\x8e" "2",           /* 低2   LOW 2  */
    "\xe4\xbd\x8e" "3",           /* 低3   LOW 3  */
    "\xe4\xbd\x8e" "4",           /* 低4   LOW 4  */
    "\xe4\xbd\x8e" "5",           /* 低5   LOW 5  */
    "\xe4\xb8\xad",               /* 中    MID    */
    "\xe9\xab\x98",               /* 高    HIGH   */
};

/* ── Offset direction ────────────────────────────────────────────────────── */
const char * const gSubMenu_SFT_D_zh[] = {
    "\xe5\x85\xb3",   /* 关  OFF */
    "+",
    "-",
};

/* ── Wide / Narrow ──────────────────────────────────────────────────────── */
const char * const gSubMenu_W_N_zh[] = {
    "\xe5\xae\xbd\xe5\xb8\xa6",   /* 宽带  WIDE   */
    "\xe7\xaa\x84\xe5\xb8\xa6",   /* 窄带  NARROW */
};

/* ── Dual-watch / RX mode ────────────────────────────────────────────────── */
const char * const gSubMenu_RXMode_zh[] = {
    "\xe5\x8d\x95\xe9\xa2\x91\xe5\xae\x88\xe5\x80\x99",                /* 单频守候  MAIN ONLY      */
    "\xe5\x8f\x8c\xe9\xa2\x91\xe5\xae\x88\xe5\x80\x99",                /* 双频守候  DUAL RX RESPOND */
    "\xe8\xb7\xa8\xe6\xae\xb5",                                        /* 跨段      CROSS BAND     */
    "\xe4\xb8\xbb\xe5\x8f\x91\xe5\x8f\x8c\xe6\x94\xb6",               /* 主发双收  MAIN TX DUAL RX */
};

/* ── Channel display mode ───────────────────────────────────────────────── */
const char * const gSubMenu_MDF_zh[] = {
    "\xe9\xa2\x91\xe7\x8e\x87",                                        /* 频率      FREQ          */
    "\xe4\xbf\xa1\xe9\x81\x93\xe5\x8f\xb7",                           /* 信道号    CHANNEL NUMBER */
    "\xe5\x90\x8d\xe7\xa7\xb0",                                        /* 名称      NAME          */
    "\xe5\x90\x8d\xe7\xa7\xb0+\xe9\xa2\x91\xe7\x8e\x87",              /* 名称+频率 NAME+FREQ      */
};

/* ── PTT ID ──────────────────────────────────────────────────────────────── */
const char * const gSubMenu_PTT_ID_zh[] = {
    "\xe5\x85\xb3",                                                    /* 关        OFF          */
    "\xe4\xb8\x8a\xe8\xa1\x8c\xe7\xa0\x81",                           /* 上行码    UP CODE       */
    "\xe4\xb8\x8b\xe8\xa1\x8c\xe7\xa0\x81",                           /* 下行码    DOWN CODE     */
    "\xe4\xb8\x8a+\xe4\xb8\x8b\n\xe8\xa1\x8c\xe7\xa0\x81",            /* 上+下行码 UP+DOWN CODE  */
    "APOLLO\nQUINDAR",
};

/* ── Power-on message ────────────────────────────────────────────────────── */
const char * const gSubMenu_PONMSG_zh[] = {
#ifdef ENABLE_FEAT_F4HWN
    "\xe5\x85\xa8\xe9\x83\xa8",   /* 全部  ALL     */
    "\xe5\xa3\xb0\xe9\x9f\xb3",   /* 声音  SOUND   */
#else
    "\xe5\x85\xa8\xe9\x83\xa8",   /* 全部  FULL    */
#endif
    "\xe6\xb6\x88\xe6\x81\xaf",   /* 消息  MESSAGE */
    "\xe7\x94\xb5\xe5\x8e\x8b",   /* 电压  VOLTAGE */
#ifdef ENABLE_FEAT_F4HWN_LOGO
    "\xe5\x9b\xbe\xe6\xa0\x87",   /* 图标  LOGO    */
#endif
    "\xe6\x97\xa0",               /* 无    NONE    */
};

/* ── Roger beep ──────────────────────────────────────────────────────────── */
const char * const gSubMenu_ROGER_zh[] = {
    "\xe5\x85\xb3",                                    /* 关    OFF   */
    "\xe7\xa1\xae\xe8\xae\xa4\xe9\x9f\xb3",           /* 确认音 ROGER */
    "MDC",
};

/* ── Factory reset scope ─────────────────────────────────────────────────── */
const char * const gSubMenu_RESET_zh[] = {
    "VFO",
    "\xe5\x85\xa8\xe9\x83\xa8",   /* 全部  ALL */
};

/* ── Battery text style ──────────────────────────────────────────────────── */
const char * const gSubMenu_BAT_TXT_zh[] = {
    "\xe6\x97\xa0",                                    /* 无    NONE    */
    "\xe7\x94\xb5\xe5\x8e\x8b",                       /* 电压  VOLTAGE */
    "\xe7\x99\xbe\xe5\x88\x86\xe6\xaf\x94",           /* 百分比 PERCENT */
};

/* ── Backlight / CTCSS TX/RX select ─────────────────────────────────────── */
const char * const gSubMenu_RX_TX_zh[] = {
    "\xe5\x85\xb3",               /* 关  OFF   */
    "\xe5\x8f\x91\xe5\xb0\x84",   /* 发射 TX   */
    "\xe6\x8e\xa5\xe6\x94\xb6",   /* 接收 RX   */
    "\xe6\x94\xb6\xe5\x8f\x91",   /* 收发 TX/RX */
};

/* ── Frequency lock bands ────────────────────────────────────────────────── */
const char * const gSubMenu_F_LOCK_zh[] = {
    "\xe9\xbb\x98\xe8\xae\xa4\n137-174\n400-470",     /* 默认      DEFAULT+       */
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
    "\xe7\xa6\x81\xe6\xad\xa2\n\xe5\x85\xa8\xe9\x83\xa8",   /* 禁止全部  DISABLE ALL */
    "\xe8\xa7\xa3\xe9\x94\x81\n\xe5\x85\xa8\xe9\x83\xa8",   /* 解锁全部  UNLOCK ALL  */
};

/* ── Navigation key orientation ─────────────────────────────────────────── */
const char * const gSubMenu_SET_NAV_zh[] = {
    "\xe5\xb7\xa6\n\xe5\x8f\xb3\nUV-K1",     /* 左右 UV-K1   LEFT/RIGHT */
    "\xe4\xb8\x8a\n\xe4\xb8\x8b\nUV-K5(8)",   /* 上下 UV-K5   UP/DOWN    */
};

#ifdef ENABLE_VOICE
/* ── Voice prompt language ────────────────────────────────────────────────── */
const char * const gSubMenu_VOICE_zh[] = {
    "\xe5\x85\xb3",               /* 关  OFF */
    "\xe4\xb8\xad\xe6\x96\x87",   /* 中文 CHI */
    "\xe8\x8b\xb1\xe6\x96\x87",   /* 英文 ENG */
};
#endif

#ifdef ENABLE_ALARM
/* ── Alarm mode ───────────────────────────────────────────────────────────── */
const char * const gSubMenu_AL_MOD_zh[] = {
    "\xe7\x8e\xb0\xe5\x9c\xba",   /* 现场  SITE */
    "\xe9\x9f\xb3\xe8\xb0\x83",   /* 音调  TONE */
};
#endif

#ifdef ENABLE_DTMF_CALLING
/* ── DTMF response mode ───────────────────────────────────────────────────── */
const char * const gSubMenu_D_RSP_zh[] = {
    "\xe6\x97\xa0\xe5\x8a\xa8\xe4\xbd\x9c",   /* 无动作  DO NOTHING */
    "\xe5\x93\x8d\xe9\x93\x83",               /* 响铃    RING       */
    "\xe5\x9b\x9e\xe5\xa4\x8d",               /* 回复    REPLY      */
    "\xe5\x8f\x8c\xe7\xab\xaf",               /* 双端    BOTH       */
};
#endif

#ifndef ENABLE_FEAT_F4HWN
/* ── Scrambler ────────────────────────────────────────────────────────────── */
const char * const gSubMenu_SCRAMBLER_zh[] = {
    "\xe5\x85\xb3",   /* 关  OFF */
    "2600Hz", "2700Hz", "2800Hz", "2900Hz",
    "3000Hz", "3100Hz", "3200Hz", "3300Hz",
    "3400Hz", "3500Hz",
};
#endif

#ifdef ENABLE_FEAT_F4HWN
/* ── F4HWN extended power levels (watt labels) ──────────────────────────── */
/* NOTE: kept in English — these are technical watt values used in compound
 *       strings like "低1\n125mW".  Translating "< 20m" etc. adds no value. */

/* ── PTT mode ─────────────────────────────────────────────────────────────── */
const char * const gSubMenu_SET_PTT_zh[] = {
    "\xe7\xbb\x8f\xe5\x85\xb8",   /* 经典  CLASSIC  */
    "\xe5\x8d\x95\xe9\x94\xae",   /* 单键  ONEPUSH  */
};

/* ── TX/EOT timeout alert ─────────────────────────────────────────────────── */
const char * const gSubMenu_SET_TOT_zh[] = {
    "\xe5\x85\xb3",               /* 关  OFF    */
    "\xe5\xa3\xb0\xe9\x9f\xb3",   /* 声音 SOUND  */
    "\xe9\x97\xaa\xe7\x83\x81",   /* 闪烁 VISUAL */
    "\xe5\x85\xa8\xe9\x83\xa8",   /* 全部 ALL    */
};

/* ── Key lock mode ────────────────────────────────────────────────────────── */
const char * const gSubMenu_SET_LCK_zh[] = {
    "\xe9\x94\x81\xe9\x94\xae",         /* 锁键      KEYS      */
    "\xe9\x94\x81\xe9\x94\xae+PTT",     /* 锁键+PTT  KEYS+PTT  */
};

/* ── S-meter / GUI style ──────────────────────────────────────────────────── */
const char * const gSubMenu_SET_MET_zh[] = {
    "\xe7\xb2\xbe\xe7\xae\x80",   /* 精简  TINY    */
    "\xe7\xbb\x8f\xe5\x85\xb8",   /* 经典  CLASSIC */
};

#ifdef ENABLE_FEAT_F4HWN_SCAN_FASTER
/* ── Scan speed ────────────────────────────────────────────────────────────── */
const char * const gSubMenu_SET_SCN_zh[] = {
    "\xe6\xad\xa3\xe5\xb8\xb8",   /* 正常  NORMAL */
    "\xe5\xbf\xab\xe9\x80\x9f",   /* 快速  FAST   */
};
#endif

#ifdef ENABLE_FEAT_F4HWN_AUDIO
/* ── FM audio profile ─────────────────────────────────────────────────────── */
const char * const gSubMenu_SET_AUD_FM_zh[] = {
    "\xe5\xb9\xb3\xe5\x9d\xa6",   /* 平坦  FLAT  */
    "\xe6\xb8\x85\xe6\x99\xb0",   /* 清晰  CLEAN */
    "\xe4\xb8\xad\xe7\xad\x89",   /* 中等  MID   */
    "\xe5\xa2\x9e\xe5\xbc\xba",   /* 增强  BOOST */
    "\xe6\x9c\x80\xe5\xbc\xba",   /* 最强  MAX   */
};

/* ── AM audio profile ─────────────────────────────────────────────────────── */
const char * const gSubMenu_SET_AUD_AM_zh[] = {
    "\xe9\x94\x90\xe5\x88\xa9",   /* 锐利  SHARP */
    "\xe6\xa0\x87\xe5\x87\x86",   /* 标准  STOCK */
    "\xe5\xbc\x80\xe6\x94\xbe",   /* 开放  OPEN  */
};
#endif

#ifdef ENABLE_FEAT_F4HWN_NARROWER
/* ── NFM bandwidth ─────────────────────────────────────────────────────────── */
const char * const gSubMenu_SET_NFM_zh[] = {
    "\xe7\xaa\x84\xe5\xb8\xa6",               /* 窄带    NARROW   */
    "\xe8\xb6\x85\xe7\xaa\x84\xe5\xb8\xa6",   /* 超窄带  NARROWER */
};
#endif

#endif /* ENABLE_FEAT_F4HWN */

/* ── Side-key / function-key actions
 *
 * IMPORTANT: element order and #ifdef guards MUST match gSubMenu_SIDEFUNCTIONS
 * in menu.c exactly — both arrays are indexed by the same gSubMenuSelection.
 * ────────────────────────────────────────────────────────────────────────── */
const char * const gSubMenu_SIDEFUNCTIONS_zh[] = {
    "\xe6\x97\xa0",                                    /* 无      NONE          */
#ifdef ENABLE_FLASHLIGHT
    "\xe9\x97\xaa\xe5\x85\x89\xe7\x81\xaf",           /* 闪光灯  FLASHLIGHT    */
#endif
    "\xe5\x8a\x9f\xe7\x8e\x87",                       /* 功率    POWER         */
    "\xe7\x9b\x91\xe5\x90\xac",                       /* 监听    MONITOR       */
    "\xe6\x89\xab\xe6\x8f\x8f",                       /* 扫描    SCAN          */
#ifdef ENABLE_VOX
    "VOX",
#endif
#ifdef ENABLE_ALARM
    "\xe6\x8a\xa5\xe8\xad\xa6",                       /* 报警    ALARM         */
#endif
#ifdef ENABLE_FMRADIO
    "\xe8\xb0\x83\xe9\xa2\x91",                       /* 调频    FM RADIO      */
#endif
#ifdef ENABLE_TX1750
    "1750Hz",
#endif
#ifdef ENABLE_REGA
    "REGA\n\xe6\x8a\xa5\xe8\xad\xa6",                 /* REGA报警 REGA ALARM   */
    "REGA\n\xe6\xb5\x8b\xe8\xaf\x95",                 /* REGA测试 REGA TEST    */
#endif
    "\xe9\x94\x81\xe9\x94\xae\xe7\x9b\x98",           /* 锁键盘  LOCK KEYPAD   */
    "VFO A\nVFO B",
    "VFO/\xe5\xad\x98",                               /* VFO/存  VFO MEM       */
    "\xe6\xa8\xa1\xe5\xbc\x8f",                       /* 模式    MODE          */
#ifdef ENABLE_BLMIN_TMP_OFF
    "\xe8\x83\x8c\xe5\x85\x89\n\xe6\x9a\x82\xe6\x9a\x97", /* 背光暂暗 BLMIN TMP OFF */
#endif
#ifdef ENABLE_FEAT_F4HWN
    "\xe6\x8e\xa5\xe6\x94\xb6\xe6\xa8\xa1\xe5\xbc\x8f",   /* 接收模式 RX MODE     */
    "\xe4\xbb\x85\xe4\xb8\xbb\xe9\xa2\x91",               /* 仅主频   MAIN ONLY   */
    "PTT",
    "\xe5\xae\xbd/\xe7\xaa\x84",                          /* 宽/窄   WIDE/NARROW  */
    "\xe9\x9d\x99\xe9\x9f\xb3",                           /* 静音    MUTE         */
    #ifdef ENABLE_FEAT_F4HWN_AUDIO
        "\xe6\x8e\xa5\xe6\x94\xb6\xe9\x9f\xb3",           /* 接收音  RxA           */
    #endif
    #ifdef ENABLE_FEAT_F4HWN_RESCUE_OPS
        "\xe9\xab\x98\xe5\x8a\x9f\xe7\x8e\x87",           /* 高功率  POWER HIGH    */
        "\xe6\xb8\x85\xe5\x81\x8f\xe7\xa7\xbb",           /* 清偏移  REMOVE OFFSET */
    #endif
    #ifdef ENABLE_FEAT_F4HWN_BEAM
        "\xe5\xae\x9a\xe5\x90\x91\xe5\xa4\xa9\xe7\xba\xbf", /* 定向天线 BEAM        */
    #endif
#endif /* ENABLE_FEAT_F4HWN */
};
