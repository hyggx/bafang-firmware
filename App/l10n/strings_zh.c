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
    /* ── Menu labels (Haige-aligned translations) ── */
    [STR_MENU_SQL]          = "\xe9\x9d\x99\xe5\x99\xaa\xe7\xad\x89\xe7\xba\xa7",       /* 静噪等级 */
    [STR_MENU_STEP]         = "\xe6\xad\xa5\xe8\xbf\x9b\xe9\xa2\x91\xe7\x8e\x87",       /* 步进频率 */
    [STR_MENU_TXP]          = "\xe5\x8f\x91\xe5\xb0\x84\xe5\x8a\x9f\xe7\x8e\x87",       /* 发射功率 */
    [STR_MENU_R_DCS]        = "\xe6\x8e\xa5\xe6\x94\xb6\xe6\x95\xb0\xe5\xad\x97\xe4\xba\x9a\xe9\x9f\xb3", /* 接收数字亚音 */
    [STR_MENU_R_CTCS]       = "\xe6\x8e\xa5\xe6\x94\xb6\xe6\xa8\xa1\xe6\x8b\x9f\xe4\xba\x9a\xe9\x9f\xb3", /* 接收模拟亚音 */
    [STR_MENU_T_DCS]        = "\xe5\x8f\x91\xe5\xb0\x84\xe6\x95\xb0\xe5\xad\x97\xe4\xba\x9a\xe9\x9f\xb3", /* 发射数字亚音 */
    [STR_MENU_T_CTCS]       = "\xe5\x8f\x91\xe5\xb0\x84\xe6\xa8\xa1\xe6\x8b\x9f\xe4\xba\x9a\xe9\x9f\xb3", /* 发射模拟亚音 */
    [STR_MENU_SFT_D]        = "\xe9\xa2\x91\xe5\xb7\xae\xe6\x96\xb9\xe5\x90\x91",       /* 频差方向 */
    [STR_MENU_OFFSET]       = "\xe9\xa2\x91\xe5\xb7\xae\xe9\xa2\x91\xe7\x8e\x87",       /* 频差频率 */
    [STR_MENU_TOT]          = "\xe5\x8f\x91\xe5\xb0\x84\xe8\xb6\x85\xe6\x97\xb6",       /* 发射超时 */
    [STR_MENU_W_N]          = "\xe4\xbf\xa1\xe9\x81\x93\xe5\xb8\xa6\xe5\xae\xbd",       /* 信道带宽 */
    [STR_MENU_SCR]          = "\xe5\x8a\xa0\xe5\xaf\x86\xe9\x80\x9a\xe8\xaf\x9d",       /* 加密通话 */
    [STR_MENU_BCL]          = "\xe9\x81\x87\xe5\xbf\x99\xe7\xa6\x81\xe5\x8f\x91",       /* 遇忙禁发 */
    [STR_MENU_TX_LOCK]      = "\xe5\x8f\x91\xe5\xb0\x84\xe9\x94\x81\xe5\xae\x9a",       /* 发射锁定 */
    [STR_MENU_MEM_CH]       = "\xe5\xad\x98\xe5\x82\xa8\xe4\xbf\xa1\xe9\x81\x93",       /* 存储信道 */
    [STR_MENU_DEL_CH]       = "\xe5\x88\xa0\xe9\x99\xa4\xe4\xbf\xa1\xe9\x81\x93",       /* 删除信道 */
    [STR_MENU_MEM_NAME]     = "\xe7\xbc\x96\xe8\xbe\x91\xe5\x90\x8d\xe7\xa7\xb0",       /* 编辑名称 */
    [STR_MENU_MDF]          = "\xe4\xbf\xa1\xe9\x81\x93\xe6\x98\xbe\xe7\xa4\xba",       /* 信道显示 */
    [STR_MENU_SAVE]         = "\xe7\x9c\x81\xe7\x94\xb5\xe6\xa8\xa1\xe5\xbc\x8f",       /* 省电模式 */
    [STR_MENU_VOX]          = "\xe5\xa3\xb0\xe6\x8e\xa7\xe5\x8f\x91\xe5\xb0\x84",       /* 声控发射 */
    [STR_MENU_ABR]          = "\xe8\x83\x8c\xe5\x85\x89\xe6\x97\xb6\xe9\x97\xb4",       /* 背光时间 */
    [STR_MENU_ABR_ON_TX_RX] = "\xe6\x94\xb6\xe5\x8f\x91\xe8\x83\x8c\xe5\x85\x89",       /* 收发背光 */
    [STR_MENU_ABR_MIN]      = "\xe8\x83\x8c\xe5\x85\x89\xe6\x9c\x80\xe6\x9a\x97",       /* 背光最暗 */
    [STR_MENU_ABR_MAX]      = "\xe8\x83\x8c\xe5\x85\x89\xe6\x9c\x80\xe4\xba\xae",       /* 背光最亮 */
    [STR_MENU_TDR]          = "\xe5\x8f\x8c\xe5\xbe\x85\xe6\xa8\xa1\xe5\xbc\x8f",       /* 双待模式 */
    [STR_MENU_BEEP]         = "\xe6\x8c\x89\xe9\x94\xae\xe9\x9f\xb3",                   /* 按键音   */
    [STR_MENU_VOICE]        = "\xe8\xaf\xad\xe9\x9f\xb3\xe6\x8f\x90\xe7\xa4\xba",       /* 语音提示 */
    [STR_MENU_SC_REV]       = "\xe6\x89\xab\xe6\x8f\x8f\xe6\x81\xa2\xe5\xa4\x8d",       /* 扫描恢复 */
    [STR_MENU_AUTOLK]       = "\xe8\x87\xaa\xe5\x8a\xa8\xe9\x94\x81\xe9\x94\xae",       /* 自动锁键 */
    [STR_MENU_LIST_CH]      = "\xe4\xbf\xa1\xe9\x81\x93\xe5\x88\x97\xe8\xa1\xa8",       /* 信道列表 */
    [STR_MENU_STE]          = "\xe5\xb0\xbe\xe9\x9f\xb3\xe6\xb6\x88\xe9\x99\xa4",       /* 尾音消除 */
    [STR_MENU_RP_STE]       = "\xe4\xb8\xad\xe7\xbb\xa7\xe5\xb0\xbe\xe9\x9f\xb3",       /* 中继尾音 */
    [STR_MENU_MIC]          = "\xe9\xba\xa6\xe5\x85\x8b\xe9\xa3\x8e\xe5\xa2\x9e\xe7\x9b\x8a",   /* 麦克风增益 */
    [STR_MENU_MIC_BAR]      = "\xe8\xaf\x9d\xe9\x9f\xb3\xe7\x94\xb5\xe5\xb9\xb3",       /* 话音电平 */
    [STR_MENU_COMPAND]      = "\xe8\xaf\xad\xe9\x9f\xb3\xe5\x8e\x8b\xe6\x89\xa9",       /* 语音压扩 */
    [STR_MENU_1_CALL]       = "\xe4\xb8\x80\xe9\x94\xae\xe5\x8d\xb3\xe5\x91\xbc",       /* 一键即呼 */
    [STR_MENU_S_LIST]       = "\xe6\x89\xab\xe6\x8f\x8f\xe5\x88\x97\xe8\xa1\xa8",       /* 扫描列表 */
    [STR_MENU_S_PRI]        = "\xe6\x89\xab\xe6\x8f\x8f\xe4\xbc\x98\xe5\x85\x88",       /* 扫描优先 */
    [STR_MENU_S_PRI_CH_1]   = "\xe4\xbc\x98\xe5\x85\x88\xe4\xbf\xa1\xe9\x81\x93" "1",          /* 优先信道1 */
    [STR_MENU_S_PRI_CH_2]   = "\xe4\xbc\x98\xe5\x85\x88\xe4\xbf\xa1\xe9\x81\x93" "2",          /* 优先信道2 */
    [STR_MENU_AL_MOD]       = "\xe6\x8a\xa5\xe8\xad\xa6",                               /* 报警     */
    [STR_MENU_ANI_ID]       = "\xe5\x8f\xb0\xe5\x8f\xb7",                               /* 台号     */
    [STR_MENU_UPCODE]       = "DTMF\xe4\xb8\x8a\xe7\xba\xbf\xe7\xa0\x81",               /* DTMF上线码 */
    [STR_MENU_DWCODE]       = "DTMF\xe4\xb8\x8b\xe7\xba\xbf\xe7\xa0\x81",               /* DTMF下线码 */
    [STR_MENU_PTT_ID]       = "PTT ID",                                                  /* PTT ID  */
    [STR_MENU_D_ST]         = "DTMF\xe4\xbe\xa7\xe9\x9f\xb3",                           /* DTMF侧音 */
    [STR_MENU_D_RSP]        = "DTMF\xe5\x93\x8d\xe5\xba\x94",                           /* DTMF响应 */
    [STR_MENU_D_HOLD]       = "DTMF\xe4\xbf\x9d\xe6\x8c\x81",                           /* DTMF保持 */
    [STR_MENU_D_PRE]        = "DTMF\xe5\x89\x8d\xe5\xaf\xbc",                           /* DTMF前导 */
    [STR_MENU_D_DCD]        = "DTMF\xe8\xa7\xa3\xe7\xa0\x81",                           /* DTMF解码 */
    [STR_MENU_D_LIST]       = "DTMF\xe8\x81\x94\xe7\xb3\xbb",                           /* DTMF联系 */
    [STR_MENU_D_LIVE_DEC]   = "DTMF\xe7\x9b\xb4\xe8\xa7\xa3",                           /* DTMF直解 */
    [STR_MENU_PONMSG]       = "\xe5\xbc\x80\xe6\x9c\xba\xe6\x98\xbe\xe7\xa4\xba",       /* 开机显示 */
    [STR_MENU_ROGER]        = "\xe5\x8f\x91\xe9\x80\x81\xe7\xbb\x93\xe6\x9d\x9f\xe9\x9f\xb3", /* 发送结束音 */
    [STR_MENU_VOL]          = "\xe7\xb3\xbb\xe7\xbb\x9f\xe4\xbf\xa1\xe6\x81\xaf",       /* 系统信息 */
    [STR_MENU_BAT_TXT]      = "\xe7\x94\xb5\xe6\xb1\xa0\xe6\x98\xbe\xe7\xa4\xba",       /* 电池显示 */
    [STR_MENU_AM]           = "\xe8\xb0\x83\xe5\x88\xb6\xe6\xa8\xa1\xe5\xbc\x8f",       /* 调制模式 */
    [STR_MENU_AM_FIX]       = "AM\xe4\xbf\xae\xe6\xad\xa3",                             /* AM修正   */
    [STR_MENU_NOAA_S]       = "NOAA",                                                    /* NOAA     */
    [STR_MENU_RESET]        = "\xe6\x81\xa2\xe5\xa4\x8d\xe5\x87\xba\xe5\x8e\x82",       /* 恢复出厂 */
    [STR_MENU_F_LOCK]       = "\xe9\x94\x81\xe5\xae\x9a\xe9\xa2\x91\xe6\xae\xb5",       /* 锁定频段 */
    [STR_MENU_200TX]        = "Tx200",                                                   /* Tx200    */
    [STR_MENU_350TX]        = "Tx350",                                                   /* Tx350    */
    [STR_MENU_500TX]        = "Tx500",                                                   /* Tx500    */
    [STR_MENU_350EN]        = "350\xe5\x90\xaf\xe7\x94\xa8",                             /* 350启用  */
    [STR_MENU_SCREN]        = "\xe9\xa2\x91\xe8\xb0\xb1\xe5\xbc\x80\xe5\x85\xb3",       /* 频谱开关 */
    [STR_MENU_F_CALI]       = "\xe9\xa2\x91\xe7\x8e\x87\xe6\xa0\xa1\xe5\x87\x86",       /* 频率校准 */
    [STR_MENU_SET_OFF]      = "\xe5\x85\xb3\xe6\x9c\xba\xe5\xae\x9a\xe6\x97\xb6",       /* 关机定时 */
    [STR_MENU_SET_PTT]      = "PTT\xe8\xae\xbe\xe7\xbd\xae",                             /* PTT设置  */
    [STR_MENU_SET_TOT]      = "\xe8\xb6\x85\xe6\x97\xb6\xe8\xae\xbe\xe7\xbd\xae",       /* 超时设置 */
    [STR_MENU_SET_EOT]      = "\xe5\x8f\x91\xe5\xb0\x84\xe7\xbb\x93\xe6\x9d\x9f\xe6\x8f\x90\xe7\xa4\xba",  /* 发射结束提示 */
    [STR_MENU_SET_CTR]      = "\xe5\xb1\x8f\xe5\xb9\x95\xe5\xaf\xb9\xe6\xaf\x94\xe5\xba\xa6",   /* 屏幕对比度 */
    [STR_MENU_SET_INV]      = "\xe5\xb1\x8f\xe5\xb9\x95\xe5\x8f\x8d\xe8\x89\xb2",       /* 屏幕反色 */
    [STR_MENU_SET_MET]      = "\xe4\xbf\xa1\xe5\x8f\xb7\xe5\xbc\xba\xe5\xba\xa6",       /* 信号强度 */
    [STR_MENU_SET_GUI]      = "\xe7\x95\x8c\xe9\x9d\xa2\xe9\xa3\x8e\xe6\xa0\xbc",       /* 界面风格 */
    [STR_MENU_SET_TMR]      = "\xe6\x94\xb6\xe5\x8f\x91\xe8\xae\xa1\xe6\x97\xb6",                   /* 收发计时 */
    [STR_MENU_SET_SCN]      = "\xe6\x89\xab\xe6\x8f\x8f\xe8\xae\xbe\xe7\xbd\xae",       /* 扫描设置 */
    [STR_MENU_SET_NFM]      = "FM\xe8\xb6\x85\xe7\xaa\x84\xe5\xb8\xa6",                  /* FM超窄带 */
    [STR_MENU_SET_VOL]      = "\xe6\x8e\xa5\xe6\x94\xb6\xe5\xa2\x9e\xe7\x9b\x8a",                     /* 接收增益 */
    [STR_MENU_SET_KEY]      = "\xe6\x8c\x89\xe9\x94\xae\xe8\xae\xbe\xe7\xbd\xae",       /* 按键设置 */
    [STR_MENU_SET_NAV]      = "\xe5\xaf\xbc\xe8\x88\xaa\xe9\x94\xae",                   /* 导航键   */
    [STR_MENU_SET_AUD]      = "\xe9\x9f\xb3\xe9\xa2\x91\xe8\xae\xbe\xe7\xbd\xae",       /* 音频设置 */
    [STR_MENU_BATCAL]       = "\xe7\x94\xb5\xe6\xb1\xa0\xe6\xa0\xa1\xe5\x87\x86",       /* 电池校准 */
    [STR_MENU_F1SHRT]       = "\xe9\x94\xae" "1\xe7\x9f\xad\xe6\x8c\x89",               /* 键1短按  */
    [STR_MENU_F1LONG]       = "\xe9\x94\xae" "1\xe9\x95\xbf\xe6\x8c\x89",               /* 键1长按  */
    [STR_MENU_F2SHRT]       = "\xe9\x94\xae" "2\xe7\x9f\xad\xe6\x8c\x89",               /* 键2短按  */
    [STR_MENU_F2LONG]       = "\xe9\x94\xae" "2\xe9\x95\xbf\xe6\x8c\x89",               /* 键2长按  */
    [STR_MENU_MLONG]        = "\xe8\x8f\x9c\xe5\x8d\x95\xe9\x95\xbf\xe6\x8c\x89",       /* 菜单长按 */
    [STR_MENU_BATTYP]       = "\xe7\x94\xb5\xe6\xb1\xa0\xe7\xb1\xbb\xe5\x9e\x8b",       /* 电池类型 */

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
