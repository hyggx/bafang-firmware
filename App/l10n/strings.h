#ifndef L10N_STRINGS_H
#define L10N_STRINGS_H

#include <stdint.h>

/* --------------------------------------------------------------------------
 * Language identifiers
 * -------------------------------------------------------------------------- */
typedef enum {
    LANG_EN = 0,   /* English (default / fallback) */
    LANG_ZH = 1,   /* Simplified Chinese            */
    LANG_COUNT
} LangId_t;

/* --------------------------------------------------------------------------
 * String identifiers — one per UI-visible string.
 *
 * Rules:
 *  - Append only; never reorder or remove (bump EEPROM_LAYOUT_VERSION if
 *    the language preference field moves).
 *  - STR_COUNT must equal the number of entries in g_str_en[] / g_str_zh[].
 *    A _Static_assert in each .c file enforces this at compile time.
 * -------------------------------------------------------------------------- */
typedef enum {

    /* ── Menu item labels (short, ≤ 6 ASCII chars on display) ── */
    STR_MENU_SQL = 0,       /* Sql        / 静噪   */
    STR_MENU_STEP,          /* Step       / 步进   */
    STR_MENU_TXP,           /* Power      / 发射功率 */
    STR_MENU_R_DCS,         /* RxDCS      / 收DCS  */
    STR_MENU_R_CTCS,        /* RxCTCS     / 收亚音  */
    STR_MENU_T_DCS,         /* TxDCS      / 发DCS  */
    STR_MENU_T_CTCS,        /* TxCTCS     / 发亚音  */
    STR_MENU_SFT_D,         /* TxODir     / 偏移方向 */
    STR_MENU_OFFSET,        /* TxOffs     / 偏移频率 */
    STR_MENU_TOT,           /* TxTOut     / 超时关断 */
    STR_MENU_W_N,           /* W/N        / 宽窄带  */
    STR_MENU_SCR,           /* Scramb     / 加密   */
    STR_MENU_BCL,           /* BusyCL     / 忙道锁  */
    STR_MENU_TX_LOCK,       /* TXLock     / 发射锁  */
    STR_MENU_MEM_CH,        /* ChSave     / 存信道  */
    STR_MENU_DEL_CH,        /* ChDele     / 删信道  */
    STR_MENU_MEM_NAME,      /* ChName     / 信道名  */
    STR_MENU_MDF,           /* ChDisp     / 信道显示 */
    STR_MENU_SAVE,          /* BatSav     / 省电   */
    STR_MENU_VOX,           /* VOX        / VOX   */
    STR_MENU_ABR,           /* BLTime     / 背光时长 */
    STR_MENU_ABR_ON_TX_RX,  /* BLTxRx     / 背光收发 */
    STR_MENU_ABR_MIN,       /* BLMin      / 背光最暗 */
    STR_MENU_ABR_MAX,       /* BLMax      / 背光最亮 */
    STR_MENU_TDR,           /* RxMode     / 双守   */
    STR_MENU_BEEP,          /* Beep       / 按键音  */
    STR_MENU_VOICE,         /* Voice      / 语音   */
    STR_MENU_SC_REV,        /* ScnRev     / 扫描恢复 */
    STR_MENU_AUTOLK,        /* KeyLck     / 自动锁键 */
    STR_MENU_LIST_CH,       /* ChList     / 信道列表 */
    STR_MENU_STE,           /* STE        / 静噪尾消 */
    STR_MENU_RP_STE,        /* RP STE     / 中继尾消 */
    STR_MENU_MIC,           /* Mic        / 麦克风  */
    STR_MENU_MIC_BAR,       /* MicBar     / 麦克风条 */
    STR_MENU_COMPAND,       /* Compnd     / 压扩   */
    STR_MENU_1_CALL,        /* 1 Call     / 单呼   */
    STR_MENU_S_LIST,        /* ScList     / 扫描列表 */
    STR_MENU_S_PRI,         /* ScPri      / 扫描优先 */
    STR_MENU_S_PRI_CH_1,    /* PriCh1     / 优先道1  */
    STR_MENU_S_PRI_CH_2,    /* PriCh2     / 优先道2  */
    STR_MENU_AL_MOD,        /* AlarmT     / 报警模式 */
    STR_MENU_ANI_ID,        /* ANI ID     / 台号   */
    STR_MENU_UPCODE,        /* UPCode     / 上码   */
    STR_MENU_DWCODE,        /* DWCode     / 下码   */
    STR_MENU_PTT_ID,        /* PTT ID     / PTT码  */
    STR_MENU_D_ST,          /* D ST       / DTMF始  */
    STR_MENU_D_RSP,         /* D Resp     / DTMF应答 */
    STR_MENU_D_HOLD,        /* D Hold     / DTMF保持 */
    STR_MENU_D_PRE,         /* D Prel     / DTMF前码 */
    STR_MENU_D_DCD,         /* D Decd     / DTMF解码 */
    STR_MENU_D_LIST,        /* D List     / DTMF列表 */
    STR_MENU_D_LIVE_DEC,    /* D Live     / 实时解码 */
    STR_MENU_PONMSG,        /* POnMsg     / 开机信息 */
    STR_MENU_ROGER,         /* Roger      / 确认音  */
    STR_MENU_VOL,           /* SysInf     / 系统信息 */
    STR_MENU_BAT_TXT,       /* BatTxt     / 电量显示 */
    STR_MENU_AM,            /* Mode       / 工作模式 */
    STR_MENU_AM_FIX,        /* AM Fix     / AM增强  */
    STR_MENU_NOAA_S,        /* NOAA-S     / NOAA扫  */
    STR_MENU_RESET,         /* Reset      / 恢复默认 */
    STR_MENU_F_LOCK,        /* F-Lock     / 频率锁定 */
    STR_MENU_200TX,         /* 200TX      / 200M发  */
    STR_MENU_350TX,         /* 350TX      / 350M发  */
    STR_MENU_500TX,         /* 500TX      / 500M发  */
    STR_MENU_350EN,         /* 350EN      / 350M开  */
    STR_MENU_SCREN,         /* ScrEn      / 频谱开关 */
    STR_MENU_F_CALI,        /* FCali      / 频率校准 */
    STR_MENU_SET_OFF,       /* SetOff     / 关机定时 */
    STR_MENU_SET_PTT,       /* SetPTT     / PTT设置  */
    STR_MENU_SET_TOT,       /* SetTOT     / 超时设置 */
    STR_MENU_SET_EOT,       /* SetEOT     / 结束超时 */
    STR_MENU_SET_CTR,       /* SetCtr     / 对比度  */
    STR_MENU_SET_INV,       /* SetInv     / 反显   */
    STR_MENU_SET_LCK,       /* SetLck     / 锁键设置 */
    STR_MENU_SET_MET,       /* SetMet     / 强度计  */
    STR_MENU_SET_GUI,       /* SetGUI     / 界面   */
    STR_MENU_SET_TMR,       /* SetTmr     / 计时器  */
    STR_MENU_SET_SCN,       /* SetScn     / 扫描设置 */
    STR_MENU_SET_NFM,       /* SetNFM     / 窄调FM  */
    STR_MENU_SET_VOL,       /* SetVol     / 音量设置 */
    STR_MENU_SET_KEY,       /* SetKey     / 按键设置 */
    STR_MENU_SET_NAV,       /* SetNav     / 导航键  */
    STR_MENU_SET_AUD,       /* SetRxA     / 音频设置 */
    STR_MENU_BATCAL,        /* BatCal     / 电压校准 */
    STR_MENU_F1SHRT,        /* F1Shrt     / F1短按  */
    STR_MENU_F1LONG,        /* F1Long     / F1长按  */
    STR_MENU_F2SHRT,        /* F2Shrt     / F2短按  */
    STR_MENU_F2LONG,        /* F2Long     / F2长按  */
    STR_MENU_MLONG,         /* M Long     / M键长按 */
    STR_MENU_BATTYP,        /* BatTyp     / 电池类型 */
    STR_MENU_LANG,          /* Lang       / 语言     */

    /* ── Common UI status strings ── */
    STR_UI_SCANNING,        /* Scanning   / 扫描中  */
    STR_UI_BUSY,            /* Busy       / 信道忙  */
    STR_UI_LOCKED,          /* Locked     / 已锁定  */
    STR_UI_VOX,             /* VOX        / VOX触发 */
    STR_UI_TX,              /* TX         / 发射   */
    STR_UI_RX,              /* RX         / 接收   */
    STR_UI_NONE,            /* None       / 无     */
    STR_UI_ON,              /* On         / 开     */
    STR_UI_OFF,             /* Off        / 关     */
    STR_UI_YES,             /* Yes        / 是     */
    STR_UI_NO,              /* No         / 否     */
    STR_UI_CONFIRM,         /* Confirm    / 确认   */
    STR_UI_CANCEL,          /* Cancel     / 取消   */
    STR_UI_SAVED,           /* Saved      / 已保存  */
    STR_UI_DELETED,         /* Deleted    / 已删除  */

    STR_COUNT   /* must be last */
} StrId_t;

/* --------------------------------------------------------------------------
 * Runtime language selection
 * -------------------------------------------------------------------------- */
extern LangId_t g_lang;

/* Return the UTF-8 string for `id` in the current language.
 * Falls back to English if the Chinese table entry is NULL or empty,
 * or if the font has not been loaded into SPI Flash yet.
 * Never returns NULL.                                                        */
const char *STR(StrId_t id);

/* Internal string tables — used only by l10n.c */
extern const char * const g_str_en[STR_COUNT];
extern const char * const g_str_zh[STR_COUNT];

#endif /* L10N_STRINGS_H */
