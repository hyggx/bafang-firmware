/* SPDX-License-Identifier: Apache-2.0
 * Simplified Chinese string table for Bafang firmware.
 *
 * These are UTF-8 encoded strings stored in firmware Flash.
 * CJK character BITMAPS are NOT stored here — they are loaded at
 * render time from the SPI Flash glyph database (App/l10n/cjk_cache.c).
 * The display layer checks each codepoint: ASCII → Latin font (in Flash);
 * CJK U+4E00–U+9FFF → glyph cache → SPI Flash lookup.
 *
 * If the SPI Flash font has not been flashed, STR() falls back to English.  */

#include "strings.h"

const char * const g_str_zh[STR_COUNT] = {
    /* ── Menu labels ── */
    [STR_MENU_SQL]          = "\xe9\x9d\x99\xe5\x99\xaa",           /* 静噪   */
    [STR_MENU_STEP]         = "\xe6\xad\xa5\xe8\xbf\x9b",           /* 步进   */
    [STR_MENU_TXP]          = "\xe5\x8f\x91\xe5\x8a\x9f\xe7\x8e\x87", /* 发功率 */
    [STR_MENU_R_DCS]        = "\xe6\x94\xb6" "DCS",               /* 收DCS  */
    [STR_MENU_R_CTCS]       = "\xe6\x94\xb6\xe4\xba\x9a\xe9\x9f\xb3", /* 收亚音 */
    [STR_MENU_T_DCS]        = "\xe5\x8f\x91" "DCS",               /* 发DCS  */
    [STR_MENU_T_CTCS]       = "\xe5\x8f\x91\xe4\xba\x9a\xe9\x9f\xb3", /* 发亚音 */
    [STR_MENU_SFT_D]        = "\xe5\x81\x8f\xe7\xa7\xbb\xe6\x96\xb9\xe5\x90\x91", /* 偏移方向 */
    [STR_MENU_OFFSET]       = "\xe5\x81\x8f\xe7\xa7\xbb\xe9\xa2\x91\xe7\x8e\x87", /* 偏移频率 */
    [STR_MENU_TOT]          = "\xe8\xb6\x85\xe6\x97\xb6\xe5\x85\xb3\xe6\x96\xad", /* 超时关断 */
    [STR_MENU_W_N]          = "\xe5\xae\xbd\xe7\xaa\x84\xe5\xb8\xa6", /* 宽窄带 */
    [STR_MENU_SCR]          = "\xe5\x8a\xa0\xe5\xaf\x86",           /* 加密   */
    [STR_MENU_BCL]          = "\xe5\xbf\x99\xe9\x81\x93\xe9\x94\x81", /* 忙道锁 */
    [STR_MENU_TX_LOCK]      = "\xe5\x8f\x91\xe5\xb0\x84\xe9\x94\x81", /* 发射锁 */
    [STR_MENU_MEM_CH]       = "\xe5\xad\x98\xe4\xbf\xa1\xe9\x81\x93", /* 存信道 */
    [STR_MENU_DEL_CH]       = "\xe5\x88\xa0\xe4\xbf\xa1\xe9\x81\x93", /* 删信道 */
    [STR_MENU_MEM_NAME]     = "\xe4\xbf\xa1\xe9\x81\x93\xe5\x90\x8d", /* 信道名 */
    [STR_MENU_MDF]          = "\xe4\xbf\xa1\xe9\x81\x93\xe6\x98\xbe\xe7\xa4\xba", /* 信道显示 */
    [STR_MENU_SAVE]         = "\xe7\x9c\x81\xe7\x94\xb5",           /* 省电   */
    [STR_MENU_VOX]          = "VOX",
    [STR_MENU_ABR]          = "\xe8\x83\x8c\xe5\x85\x89\xe6\x97\xb6\xe9\x95\xbf", /* 背光时长 */
    [STR_MENU_ABR_ON_TX_RX] = "\xe8\x83\x8c\xe5\x85\x89\xe6\x94\xb6\xe5\x8f\x91", /* 背光收发 */
    [STR_MENU_ABR_MIN]      = "\xe8\x83\x8c\xe5\x85\x89\xe6\x9c\x80\xe6\x9a\x97", /* 背光最暗 */
    [STR_MENU_ABR_MAX]      = "\xe8\x83\x8c\xe5\x85\x89\xe6\x9c\x80\xe4\xba\xae", /* 背光最亮 */
    [STR_MENU_TDR]          = "\xe5\x8f\x8c\xe5\xae\x88",           /* 双守   */
    [STR_MENU_BEEP]         = "\xe6\x8c\x89\xe9\x94\xae\xe9\x9f\xb3", /* 按键音 */
    [STR_MENU_VOICE]        = "\xe8\xaf\xad\xe9\x9f\xb3",           /* 语音   */
    [STR_MENU_SC_REV]       = "\xe6\x89\xab\xe6\x8f\x8f\xe6\x81\xa2\xe5\xa4\x8d", /* 扫描恢复 */
    [STR_MENU_AUTOLK]       = "\xe8\x87\xaa\xe5\x8a\xa8\xe9\x94\x81\xe9\x94\xae", /* 自动锁键 */
    [STR_MENU_LIST_CH]      = "\xe4\xbf\xa1\xe9\x81\x93\xe5\x88\x97\xe8\xa1\xa8", /* 信道列表 */
    [STR_MENU_STE]          = "\xe9\x9d\x99\xe5\x99\xaa\xe5\xb0\xbe\xe6\xb6\x88", /* 静噪尾消 */
    [STR_MENU_RP_STE]       = "\xe4\xb8\xad\xe7\xbb\xa7\xe5\xb0\xbe\xe6\xb6\x88", /* 中继尾消 */
    [STR_MENU_MIC]          = "\xe9\xba\xa6\xe5\x85\x8b\xe9\xa3\x8e", /* 麦克风 */
    [STR_MENU_MIC_BAR]      = "\xe9\xba\xa6\xe5\x85\x8b\xe9\xa3\x8e\xe6\x9d\xa1", /* 麦克风条 */
    [STR_MENU_COMPAND]      = "\xe5\x8e\x8b\xe6\x89\xa9",           /* 压扩   */
    [STR_MENU_1_CALL]       = "\xe5\x8d\x95\xe5\x91\xbc",           /* 单呼   */
    [STR_MENU_S_LIST]       = "\xe6\x89\xab\xe6\x8f\x8f\xe5\x88\x97\xe8\xa1\xa8", /* 扫描列表 */
    [STR_MENU_S_PRI]        = "\xe6\x89\xab\xe6\x8f\x8f\xe4\xbc\x98\xe5\x85\x88", /* 扫描优先 */
    [STR_MENU_S_PRI_CH_1]   = "\xe4\xbc\x98\xe5\x85\x88\xe9\x81\x93" "1", /* 优先道1 */
    [STR_MENU_S_PRI_CH_2]   = "\xe4\xbc\x98\xe5\x85\x88\xe9\x81\x93" "2", /* 优先道2 */
    [STR_MENU_AL_MOD]       = "\xe6\x8a\xa5\xe8\xad\xa6\xe6\xa8\xa1\xe5\xbc\x8f", /* 报警模式 */
    [STR_MENU_ANI_ID]       = "\xe5\x8f\xb0\xe5\x8f\xb7",           /* 台号   */
    [STR_MENU_UPCODE]       = "\xe4\xb8\x8a\xe7\xa0\x81",           /* 上码   */
    [STR_MENU_DWCODE]       = "\xe4\xb8\x8b\xe7\xa0\x81",           /* 下码   */
    [STR_MENU_PTT_ID]       = "PTT\xe7\xa0\x81",                    /* PTT码  */
    [STR_MENU_D_ST]         = "DTMF\xe5\xa7\x8b",                   /* DTMF始 */
    [STR_MENU_D_RSP]        = "DTMF\xe5\xba\x94\xe7\xad\x94",       /* DTMF应答 */
    [STR_MENU_D_HOLD]       = "DTMF\xe4\xbf\x9d\xe6\x8c\x81",       /* DTMF保持 */
    [STR_MENU_D_PRE]        = "DTMF\xe5\x89\x8d\xe7\xa0\x81",       /* DTMF前码 */
    [STR_MENU_D_DCD]        = "DTMF\xe8\xa7\xa3\xe7\xa0\x81",       /* DTMF解码 */
    [STR_MENU_D_LIST]       = "DTMF\xe5\x88\x97\xe8\xa1\xa8",       /* DTMF列表 */
    [STR_MENU_D_LIVE_DEC]   = "\xe5\xae\x9e\xe6\x97\xb6\xe8\xa7\xa3\xe7\xa0\x81", /* 实时解码 */
    [STR_MENU_PONMSG]       = "\xe5\xbc\x80\xe6\x9c\xba\xe4\xbf\xa1\xe6\x81\xaf", /* 开机信息 */
    [STR_MENU_ROGER]        = "\xe7\xa1\xae\xe8\xae\xa4\xe9\x9f\xb3", /* 确认音 */
    [STR_MENU_VOL]          = "\xe7\xb3\xbb\xe7\xbb\x9f\xe4\xbf\xa1\xe6\x81\xaf", /* 系统信息 */
    [STR_MENU_BAT_TXT]      = "\xe7\x94\xb5\xe9\x87\x8f\xe6\x98\xbe\xe7\xa4\xba", /* 电量显示 */
    [STR_MENU_AM]           = "\xe5\xb7\xa5\xe4\xbd\x9c\xe6\xa8\xa1\xe5\xbc\x8f", /* 工作模式 */
    [STR_MENU_AM_FIX]       = "AM\xe5\xa2\x9e\xe5\xbc\xba",         /* AM增强 */
    [STR_MENU_NOAA_S]       = "NOAA\xe6\x89\xab",                   /* NOAA扫 */
    [STR_MENU_RESET]        = "\xe6\x81\xa2\xe5\xa4\x8d\xe9\xbb\x98\xe8\xae\xa4", /* 恢复默认 */
    [STR_MENU_F_LOCK]       = "\xe9\xa2\x91\xe7\x8e\x87\xe9\x94\x81\xe5\xae\x9a", /* 频率锁定 */
    [STR_MENU_200TX]        = "200M\xe5\x8f\x91",                   /* 200M发 */
    [STR_MENU_350TX]        = "350M\xe5\x8f\x91",                   /* 350M发 */
    [STR_MENU_500TX]        = "500M\xe5\x8f\x91",                   /* 500M发 */
    [STR_MENU_350EN]        = "350M\xe5\xbc\x80",                   /* 350M开 */
    [STR_MENU_SCREN]        = "\xe9\xa2\x91\xe8\xb0\xb1\xe5\xbc\x80\xe5\x85\xb3", /* 频谱开关 */
    [STR_MENU_F_CALI]       = "\xe9\xa2\x91\xe7\x8e\x87\xe6\xa0\xa1\xe5\x87\x86", /* 频率校准 */
    [STR_MENU_SET_OFF]      = "\xe5\x85\xb3\xe6\x9c\xba\xe5\xae\x9a\xe6\x97\xb6", /* 关机定时 */
    [STR_MENU_SET_PWR]      = "\xe5\x8a\x9f\xe7\x8e\x87\xe8\xae\xbe\xe7\xbd\xae", /* 功率设置 */
    [STR_MENU_SET_PTT]      = "PTT\xe8\xae\xbe\xe7\xbd\xae",        /* PTT设置 */
    [STR_MENU_SET_TOT]      = "\xe8\xb6\x85\xe6\x97\xb6\xe8\xae\xbe\xe7\xbd\xae", /* 超时设置 */
    [STR_MENU_SET_EOT]      = "\xe7\xbb\x93\xe6\x9d\x9f\xe8\xb6\x85\xe6\x97\xb6", /* 结束超时 */
    [STR_MENU_SET_CTR]      = "\xe5\xaf\xb9\xe6\xaf\x94\xe5\xba\xa6", /* 对比度 */
    [STR_MENU_SET_INV]      = "\xe5\x8f\x8d\xe6\x98\xbe",           /* 反显   */
    [STR_MENU_SET_LCK]      = "\xe9\x94\x81\xe9\x94\xae\xe8\xae\xbe\xe7\xbd\xae", /* 锁键设置 */
    [STR_MENU_SET_MET]      = "\xe4\xbf\xa1\xe5\x8f\xb7\xe5\xbc\xba\xe5\xba\xa6", /* 信号强度 */
    [STR_MENU_SET_GUI]      = "\xe7\x95\x8c\xe9\x9d\xa2",           /* 界面   */
    [STR_MENU_SET_TMR]      = "\xe8\xae\xa1\xe6\x97\xb6\xe5\x99\xa8", /* 计时器 */
    [STR_MENU_SET_SCN]      = "\xe6\x89\xab\xe6\x8f\x8f\xe8\xae\xbe\xe7\xbd\xae", /* 扫描设置 */
    [STR_MENU_SET_NFM]      = "\xe7\xaa\x84\xe8\xb0\x83" "FM",    /* 窄调FM */
    [STR_MENU_SET_VOL]      = "\xe9\x9f\xb3\xe9\x87\x8f\xe8\xae\xbe\xe7\xbd\xae", /* 音量设置 */
    [STR_MENU_SET_KEY]      = "\xe6\x8c\x89\xe9\x94\xae\xe8\xae\xbe\xe7\xbd\xae", /* 按键设置 */
    [STR_MENU_SET_NAV]      = "\xe5\xaf\xbc\xe8\x88\xaa\xe9\x94\xae", /* 导航键 */
    [STR_MENU_SET_AUD]      = "\xe9\x9f\xb3\xe9\xa2\x91\xe8\xae\xbe\xe7\xbd\xae", /* 音频设置 */
    [STR_MENU_BATCAL]       = "\xe7\x94\xb5\xe5\x8e\x8b\xe6\xa0\xa1\xe5\x87\x86", /* 电压校准 */
    [STR_MENU_F1SHRT]       = "F1\xe7\x9f\xad\xe6\x8c\x89",         /* F1短按 */
    [STR_MENU_F1LONG]       = "F1\xe9\x95\xbf\xe6\x8c\x89",         /* F1长按 */
    [STR_MENU_F2SHRT]       = "F2\xe7\x9f\xad\xe6\x8c\x89",         /* F2短按 */
    [STR_MENU_F2LONG]       = "F2\xe9\x95\xbf\xe6\x8c\x89",         /* F2长按 */
    [STR_MENU_MLONG]        = "M\xe9\x95\xbf\xe6\x8c\x89",          /* M长按  */
    [STR_MENU_BATTYP]       = "\xe7\x94\xb5\xe6\xb1\xa0\xe7\xb1\xbb\xe5\x9e\x8b", /* 电池类型 */

    /* ── Common UI strings ── */
    [STR_UI_SCANNING]       = "\xe6\x89\xab\xe6\x8f\x8f\xe4\xb8\xad", /* 扫描中 */
    [STR_UI_BUSY]           = "\xe4\xbf\xa1\xe9\x81\x93\xe5\xbf\x99", /* 信道忙 */
    [STR_UI_LOCKED]         = "\xe5\xb7\xb2\xe9\x94\x81\xe5\xae\x9a", /* 已锁定 */
    [STR_UI_VOX]            = "VOX",
    [STR_UI_TX]             = "\xe5\x8f\x91\xe5\xb0\x84",           /* 发射   */
    [STR_UI_RX]             = "\xe6\x8e\xa5\xe6\x94\xb6",           /* 接收   */
    [STR_UI_NONE]           = "\xe6\x97\xa0",                       /* 无     */
    [STR_UI_ON]             = "\xe5\xbc\x80",                       /* 开     */
    [STR_UI_OFF]            = "\xe5\x85\xb3",                       /* 关     */
    [STR_UI_YES]            = "\xe6\x98\xaf",                       /* 是     */
    [STR_UI_NO]             = "\xe5\x90\xa6",                       /* 否     */
    [STR_UI_CONFIRM]        = "\xe7\xa1\xae\xe8\xae\xa4",           /* 确认   */
    [STR_UI_CANCEL]         = "\xe5\x8f\x96\xe6\xb6\x88",           /* 取消   */
    [STR_UI_SAVED]          = "\xe5\xb7\xb2\xe4\xbf\x9d\xe5\xad\x98", /* 已保存 */
    [STR_UI_DELETED]        = "\xe5\xb7\xb2\xe5\x88\xa0\xe9\x99\xa4", /* 已删除 */
    [STR_MENU_LANG]         = "\xe8\xaf\xad\xe8\xa8\x80",           /* 语言   */
};

_Static_assert(sizeof(g_str_zh) / sizeof(g_str_zh[0]) == STR_COUNT,
               "g_str_zh size does not match STR_COUNT");
