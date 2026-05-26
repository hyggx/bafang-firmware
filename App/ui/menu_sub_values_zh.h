/* SPDX-License-Identifier: Apache-2.0
 * Extern declarations for simplified Chinese sub-menu value string arrays.
 * Included by menu.c; definitions are in menu_sub_values_zh.c.
 *
 * The #ifdef guards mirror those in menu.c / menu_sub_values_zh.c so that
 * every zh array has exactly the same element count as its English twin.
 */

#ifndef MENU_SUB_VALUES_ZH_H
#define MENU_SUB_VALUES_ZH_H

extern const char * const gSubMenu_OFF_ON_zh[];
extern const char * const gSubMenu_TXP_zh[];
extern const char * const gSubMenu_SFT_D_zh[];
extern const char * const gSubMenu_W_N_zh[];
extern const char * const gSubMenu_RXMode_zh[];
extern const char * const gSubMenu_MDF_zh[];
extern const char * const gSubMenu_PTT_ID_zh[];
extern const char * const gSubMenu_PONMSG_zh[];
extern const char * const gSubMenu_ROGER_zh[];
extern const char * const gSubMenu_RESET_zh[];
extern const char * const gSubMenu_BAT_TXT_zh[];
extern const char * const gSubMenu_RX_TX_zh[];
extern const char * const gSubMenu_F_LOCK_zh[];
extern const char * const gSubMenu_SET_NAV_zh[];
extern const char * const gSubMenu_SIDEFUNCTIONS_zh[];

#ifdef ENABLE_VOICE
extern const char * const gSubMenu_VOICE_zh[];
#endif

#ifdef ENABLE_ALARM
extern const char * const gSubMenu_AL_MOD_zh[];
#endif

#ifdef ENABLE_DTMF_CALLING
extern const char * const gSubMenu_D_RSP_zh[];
#endif

#ifndef ENABLE_FEAT_F4HWN
extern const char * const gSubMenu_SCRAMBLER_zh[];
#endif

#ifdef ENABLE_FEAT_F4HWN
extern const char * const gSubMenu_SET_PTT_zh[];
extern const char * const gSubMenu_SET_TOT_zh[];
extern const char * const gSubMenu_SET_MET_zh[];
#ifdef ENABLE_FEAT_F4HWN_SCAN_FASTER
extern const char * const gSubMenu_SET_SCN_zh[];
#endif
#ifdef ENABLE_FEAT_F4HWN_AUDIO
extern const char * const gSubMenu_SET_AUD_FM_zh[];
extern const char * const gSubMenu_SET_AUD_AM_zh[];
#endif
#ifdef ENABLE_FEAT_F4HWN_NARROWER
extern const char * const gSubMenu_SET_NFM_zh[];
#endif
#endif /* ENABLE_FEAT_F4HWN */

#endif /* MENU_SUB_VALUES_ZH_H */
