#ifndef __CUST_PANTECH_SOUND_H__
#define __CUST_PANTECH_SOUND_H__

/* #####################  COMMON Feature ################################################*/
#define FEATURE_PANTECH_SND /* Pantech internal patch feature */
#define FEATURE_PANTECH_SND_QCOM_CR /* Qualcomm CR patch feature */
#define FEATURE_PANTECH_SND_DEBUG /* All sound related debug messages enable */
#if defined(FEATURE_PANTECH_SND_DEBUG)
#define VERY_VERBOSE_LOGGING /* ALOGVV message enable of AudioFliger.cpp, AuddioPolicyManagerBase.cpp and AuddioPolicyManagerALSA.cpp */
#endif
#define FEATURE_PANTECH_SND_ENFORCED_HEADSET /* For ear protection WCD93xx gain adjust of headset in enforced audible */
#define FEATURE_PANTECH_SND_VOICE_LOOPBACK /* For Tx ACDB separation of VOICE loopback test */

 /* #####################  DOMESTIC Feature ################################################*/
#if defined(T_EF56S) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L) || defined(T_EF63S) || defined(T_EF63K) || defined(T_EF63L) || defined(T_EF65S) // DOMESTIC
#define FEATURE_PANTECH_SND_DOMESTIC
#define FEATURE_PANTECH_SND_AUTOANSWER_RX_MUTE /* For autoanswer Rx mute on/off */
#define FEATURE_PANTECH_SND_TX_ACDB_SEPARATION /* For Voice Recognition & Camcorder Handset/Headset TxACDB separation */
#define FEATURE_PANTECH_SND_EXT_VOL_UP /* For Extreme Volume Up when receiving a call */
#define FEATURE_PANTECH_SND_VT_VOLTE_ACDB_SEPARATION /* For VT and VoLTE ACDB separation */
#define FEATURE_PANTECH_SND_VOLTE /* For VoLTE related modification */
#define FEATURE_PANTECH_SND_VOLTE_EQ /* For VoLTE EQ related modification */
#define FEATURE_PANTECH_SND_OFFLOAD /* For OFFLOAD issue fix */
#define FEATURE_PANTECH_SND_BT_NREC /* For BT NREC(AT+NREC) function */
#define FEATURE_PANTECH_SND_SHUTDOWN_SOUND /* For shutdown sound playback */
#define FEATURE_PANTECH_SND_FLAC /* 20130809 jhsong : possible playback flac contents for lpa player */

#if defined(T_EF56S) || defined(T_EF59S) || defined(T_EF60S) || defined(T_EF63S) || defined(T_EF65S) // SKT models specific feature
#define FEATURE_PANTECH_SND_SKT
#define FEATURE_PANTECH_SND_BOOT_SOUND /* For bootup sound playback */
#elif defined(T_EF59K) || defined(T_EF61K) || defined(T_EF63K) // KT models specific feature
#define FEATURE_PANTECH_SND_KT
#elif defined(T_EF59L) || defined(T_EF62L) || defined(T_EF63L) // LGU+ models specific feature
#define FEATURE_PANTECH_SND_LGT
#define FEATURE_PANTECH_SND_LGU_RMS /* For LGU+ RMS service */
#endif

#if defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF63S) || defined(T_EF63K) || defined(T_EF63L) || defined(T_EF65S) /* EF59/EF63 Series(Dual MIC) specific feature */
#define FEATURE_PANTECH_SND_SUBMIC_LOOPBACK /* For SubMic Loopback test */
#endif

#if defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L) /* EF60 series specific feature */
#define FEATURE_PANTECH_SND_PIEZO_TEST /* For Piezo Speaker Test */
#endif

#if defined(T_EF63S) || defined(T_EF63K) || defined(T_EF63L) || defined(T_EF65S) /* Dual MIC Recording for EF63 Series */
// #define FEATURE_PANTECH_SND_DUALMIC_REC
#define FEATURE_PANTECH_SND_NR /* For NR support */
#endif

#if defined(T_EF62L) // EF62L will only use QSOUND effect
#define FEATURE_PANTECH_SND_QSOUND /* For QSound Effect solution(QFX, QVSS, QXV) */
#else // Other models except EF62L wil use NXP effect
#define FEATURE_PANTECH_SND_NXP /* For NXP Sound Effect solution(Sound Experience) */
#define FEATURE_PANTECH_SND_NXP_LM /* For NXP Sound LM Effect solution(Sound Experience) */
#endif

#if defined(T_EF63S) || defined(T_EF65S) // EF63S will only use below feature
#define FEATURE_PANTECH_SND_INCALL_MUSIC /* For SKT near-end rx mixing with voice uplink for far-end */
#define FEATURE_PANTECH_SND_LGU_RMS /* For LGU+ RMS service and SKT */
#endif

#if defined(T_EF65S)
#define FEATURE_PANTECH_SND_SKT_CHARGING_MODE /* For SKT Charging Mode */
#endif

 /* #####################  VZW Feature ################################################*/
#elif defined(T_XXXX) // VZW
#define FEATURE_PANTECH_SND_ABROAD
#define FEATURE_PANTECH_SND_VZW
#define FEATURE_PANTECH_SND_QSOUND /* For QSound Effect solution(QFX, QVSS, QXV) */
#define FEATURE_PANTECH_SND_LPA /* For Tunnel or NT LPA issue fix */
#define FEATURE_PANTECH_SND_BT_GROUPING /* For BT NAC certification */
#define FEATURE_PANTECH_SND_BT_ECNR /* For BT SPEC ECNR disable(AT+NREC) function */
#define FEATURE_PANTECH_SND_ELECTOVOX /* For Transo NR function */
#define FEATURE_PANTECH_SND_BOOT_SOUND /* For bootup sound playback */
#define FEATURE_PANTECH_SND_SHUTDOWN_SOUND /* For shutdown sound playback */

 /* #####################  ATT Feature ################################################*/
#elif defined(T_XXXX) // ATT
#define FEATURE_PANTECH_SND_ABROAD
#define FEATURE_PANTECH_SND_ATT
#define FEATURE_PANTECH_SND_QSOUND /* For QSound Effect solution(QFX, QVSS, QXV) */
#define FEATURE_PANTECH_SND_LPA /* For Tunnel or NT LPA issue fix */
#define FEATURE_PANTECH_SND_BT_GROUPING /* For BT NAC certification */
#define FEATURE_PANTECH_SND_BT_ECNR /* For BT SPEC ECNR disable(AT+NREC) function */
#define FEATURE_PANTECH_SND_VOC_LOOPBACK /* For submic loop back */
#define FEATURE_PANTECH_SND_BOOT_SOUND /* For bootup sound playback */

 /* #####################  KDDI Feature ################################################*/
#elif defined(T_XXXX) // KDDI
#define FEATURE_PANTECH_SND_PMC /* For PMC Specific requirements */
#define FEATURE_PANTECH_SND_AUTOANSWER_RX_MUTE /* For autoanswer Rx mute on/off */
#define FEATURE_PANTECH_SND_TX_ACDB_SEPARATION /* For Voice Recognition & Camcorder Handset/Headset TxACDB separation */

#else
//    #error "FEATURE_PANTECH_SND ? DOMESTIC or ABROAD"
#endif

#endif /* __CUST_PANTECH_SOUND_H__ */
