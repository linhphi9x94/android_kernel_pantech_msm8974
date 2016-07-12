/************************************************************************/
/*  Copyright (c) 2004-2011 NXP Software. All rights are reserved.      */
/*  Reproduction in whole or in part is prohibited without the prior    */
/*  written consent of the copyright owner.                             */
/*                                                                      */
/*  This software and any compilation or derivative thereof is and      */
/*  shall remain the proprietary information of NXP Software and is     */
/*  highly confidential in nature. Any and all use hereof is restricted */
/*  and is subject to the terms and conditions set forth in the         */
/*  software license agreement concluded with NXP Software.             */
/*                                                                      */
/*  Under no circumstances is this software or any derivative thereof   */
/*  to be combined with any Open Source Software in any way or          */
/*  licensed under any Open License Terms without the express prior     */
/*  written  permission of NXP Software.                                */
/*                                                                      */
/*  For the purpose of this clause, the term Open Source Software means */
/*  any software that is licensed under Open License Terms. Open        */
/*  License Terms means terms in any license that require as a          */
/*  condition of use, modification and/or distribution of a work        */
/*                                                                      */
/*           1. the making available of source code or other materials  */
/*              preferred for modification, or                          */
/*                                                                      */
/*           2. the granting of permission for creating derivative      */
/*              works, or                                               */
/*                                                                      */
/*           3. the reproduction of certain notices or license terms    */
/*              in derivative works or accompanying documentation, or   */
/*                                                                      */
/*           4. the granting of a royalty-free license to any party     */
/*              under Intellectual Property Rights                      */
/*                                                                      */
/*  regarding the work and/or any work that contains, is combined with, */
/*  requires or otherwise is based on the work.                         */
/*                                                                      */
/*  This software is provided for ease of recompilation only.           */
/*  Modification and reverse engineering of this software are strictly  */
/*  prohibited.                                                         */
/*                                                                      */
/************************************************************************/

/****************************************************************************************

     $Author: nxp27078 $
     $Revision: 22069 $
     $Date: 2012-08-11 01:51:45 +0900 (Åä, 11 8 2012) $

*****************************************************************************************/

/****************************************************************************************/
/*                                                                                      */
/*  Header file defining the standard LifeVibes types for use in the application layer  */
/*  interface of all LifeVibes modules                                                  */
/*                                                                                      */
/****************************************************************************************/

#ifndef LVM_TYPES_H
#define LVM_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************************/
/*                                                                                      */
/*  definitions                                                                         */
/*                                                                                      */
/****************************************************************************************/

#define LVM_NULL                0                   /* NULL pointer */

#define LVM_TRUE                1                   /* Booleans */
#define LVM_FALSE               0

#define LVM_MAXINT_8            127                 /* Maximum positive integer size */
#define LVM_MAXINT_16           32767
#define LVM_MAXINT_32           2147483647
#define LVM_MAXUINT_32          4294967295U
#define LVM_MAXENUM             2147483647

#define LVM_MODULEID_MASK       0xFF00              /* Mask to extract the calling module ID from callbackId */
#define LVM_EVENTID_MASK        0x00FF              /* Mask to extract the callback event from callbackId */

/* Memory table*/
#define LVM_MEMREGION_PERSISTENT_SLOW_DATA      0   /* Offset to the instance memory region */
#define LVM_MEMREGION_PERSISTENT_FAST_DATA      1   /* Offset to the persistent data memory region */
#define LVM_MEMREGION_PERSISTENT_FAST_COEF      2   /* Offset to the persistent coefficient memory region */
#define LVM_MEMREGION_TEMPORARY_FAST            3   /* Offset to temporary memory region */

#define LVM_NR_MEMORY_REGIONS                   4   /* Number of memory regions */

#define LVC_MAX_NAME            256
#define LVC_MAX_UNIT            16
#define LVC_MAX_DESCRIPTION        1024

#define LVM_PARAMETER_EXTENDED

/****************************************************************************************/
/*                                                                                      */
/*  Basic types                                                                         */
/*                                                                                      */
/****************************************************************************************/

#if defined(CORE_TIC64)
typedef     char                LVM_CHAR;           /* ASCII character */

typedef     char                LVM_INT8;           /* Signed 8-bit word */
typedef     unsigned char       LVM_UINT8;          /* Unsigned 8-bit word */

typedef     short               LVM_INT16;          /* Signed 16-bit word */
typedef     unsigned short      LVM_UINT16;         /* Unsigned 16-bit word */

typedef     int                 LVM_INT32;          /* Signed 32-bit word */
typedef     unsigned int        LVM_UINT32;         /* Unsigned 32-bit word */
#else
#if (  defined(VARIANT_FRAC16) || defined (VARIANT_REFREAL16)   )
typedef     long                DATATYPE;

typedef     char                LVM_CHAR;           /* ASCII character */

typedef     char                LVM_INT8;           /* Signed 8-bit word */
typedef     unsigned char       LVM_UINT8;          /* Unsigned 8-bit word */

typedef     long                LVM_INT16;          /* Signed 16-bit word */
typedef     long                LVM_UINT16;         /* Unsigned 16-bit word */

typedef     long                LVM_INT32;          /* Signed 32-bit word */
typedef     long                LVM_UINT32;         /* Unsigned 32-bit word */
#else
typedef     char                LVM_CHAR;           /* ASCII character */

typedef     char                LVM_INT8;           /* Signed 8-bit word */
typedef     unsigned char       LVM_UINT8;          /* Unsigned 8-bit word */

typedef     short               LVM_INT16;          /* Signed 16-bit word */
typedef     unsigned short      LVM_UINT16;         /* Unsigned 16-bit word */

typedef     long                LVM_INT32;          /* Signed 32-bit word */
typedef     unsigned long       LVM_UINT32;         /* Unsigned 32-bit word */
#endif
#endif


/****************************************************************************************/
/*                                                                                      */
/*  Standard Enumerated types                                                           */
/*                                                                                      */
/****************************************************************************************/

/* Operating mode */
typedef enum
{
    LVM_MODE_OFF    = 0,
    LVM_MODE_ON     = 1,
    LVM_MODE_DUMMY  = LVM_MAXENUM
} LVM_Mode_en;


/* Format */
typedef enum
{
    LVM_STEREO          = 0,
    LVM_MONOINSTEREO    = 1,
    LVM_MONO            = 2,
    LVM_5DOT1           = 3,
    LVM_7DOT1           = 4,
    LVM_SOURCE_DUMMY    = LVM_MAXENUM
} LVM_Format_en;

/* Format */
typedef enum
{
    LVM_SPEAKER_MONO    = 0,
    LVM_SPEAKER_STEREO  = 1,
    LVM_SPEAKER_DUMMY   = LVM_MAXENUM
} LVM_SpeakerType_en;


/* Word length */
typedef enum
{
    LVM_16_BIT      = 0,
    LVM_32_BIT      = 1,
    LVM_WORDLENGTH_DUMMY = LVM_MAXENUM
} LVM_WordLength_en;


/* LVM sampling rates */
typedef enum
{
    LVM_FS_8000  = 0,
    LVM_FS_11025 = 1,
    LVM_FS_12000 = 2,
    LVM_FS_16000 = 3,
    LVM_FS_22050 = 4,
    LVM_FS_24000 = 5,
    LVM_FS_32000 = 6,
    LVM_FS_44100 = 7,
    LVM_FS_48000 = 8,
    LVM_FS_INVALID = LVM_MAXENUM-1,
    LVM_FS_DUMMY = LVM_MAXENUM
} LVM_Fs_en;


/* Reset Types */
typedef enum
{
    LVM_RESET_SOFT    = 0,
    LVM_RESET_HARD    = 1,
    LVM_RESET_DUMMY   = LVM_MAXENUM
} LVM_ResetType_en;

/* Memory Types */
typedef enum
{
    LVM_PERSISTENT_SLOW_DATA    = LVM_MEMREGION_PERSISTENT_SLOW_DATA,
    LVM_PERSISTENT_FAST_DATA    = LVM_MEMREGION_PERSISTENT_FAST_DATA,
    LVM_PERSISTENT_FAST_COEF    = LVM_MEMREGION_PERSISTENT_FAST_COEF,
    LVM_TEMPORARY_FAST          = LVM_MEMREGION_TEMPORARY_FAST,
    LVM_MEMORYTYPE_DUMMY        = LVM_MAXENUM
} LVM_MemoryTypes_en;

/* Organs */
typedef enum
{
    LVM_PROFILE_DEFAULT            = 0,
    LVM_PROFILE_HANDSET            = 1,
    LVM_PROFILE_HEADSET            = 2,
    LVM_PROFILE_CARKIT            = 3,
    LVM_PROFILE_BLUETOOTH        = 4,
    LVM_PROFILE_ACCESSORY1        = 5,
    LVM_PROFILE_ACCESSORY2        = 6,
    LVM_PROFILE_ACCESSORY3        = 7,
    LVM_PROFILE_ACCESSORY4        = 8,
    LVM_PROFILE_ACCESSORY5        = 9,
    LVM_PROFILE_ACCESSORY6        = 10,
    LVM_PROFILE_COUNT            = 11,
    LVM_PROFILE_DUMMY              = LVM_MAXENUM
} LVM_Profile_en;

/* Memory region definition */
typedef struct
{
    LVM_UINT32                  Size;                   /* Region size in bytes */
    LVM_MemoryTypes_en          Type;                   /* Region type */
    void                        *pBaseAddress;          /* Pointer to the region base address */
} LVM_MemoryRegion_st;


/* Memory table containing the region definitions */
typedef struct
{
    LVM_MemoryRegion_st         Region[LVM_NR_MEMORY_REGIONS];  /* One definition for each region */
} LVM_MemoryTable_st;

/* Beats Per Minute Structure */
typedef struct
{
    LVM_INT16                   ShortTermMinimum;       /* Beats per minute in Q9.6 format */
    LVM_INT16                   ShortTermAverage;       /* Beats per minute in Q9.6 format */
    LVM_INT16                   ShortTermMaximum;       /* Beats per minute in Q9.6 format */

    LVM_INT16                   Confidence;             /* Beat confidence level: 0 = no confidence, 32767 = maximum confidence */
    LVM_INT16                   Strength;               /* Beat strength level:   0 = no beat, 32767 = maximum strength beat */
    LVM_INT16                   LongTermMinimum;        /* Beats per minute in Q9.6 format */
    LVM_INT16                   LongTermAverage;        /* Beats per minute in Q9.6 format */
    LVM_INT16                   LongTermMaximum;        /* Beats per minute in Q9.6 format */

} LVM_BPMModuleStats_st;


typedef struct
{
    LVM_INT32     MinValue;
    LVM_INT32     MaxValue;
    LVM_CHAR     Name[LVC_MAX_NAME];
    LVM_INT32    Resolution;

    #ifdef LVM_PARAMETER_EXTENDED
    LVM_INT32     DefaultValue[LVM_PROFILE_COUNT];
    LVM_CHAR     Description[LVC_MAX_DESCRIPTION];
    LVM_CHAR     Unit[LVC_MAX_UNIT];
    LVM_Mode_en    Visibility;
    #endif

}LVM_Parameter_st;

/****************************************************************************************/
/*                                                                                      */
/*  Standard Function Prototypes                                                        */
/*                                                                                      */
/****************************************************************************************/
typedef LVM_INT32 (*LVM_Callback)(void          *pCallbackData,     /* Pointer to the callback data structure */
                                  void          *pGeneralPurpose,   /* General purpose pointer (e.g. to a data structure needed in the callback) */
                                  LVM_INT16     GeneralPurpose );   /* General purpose variable (e.g. to be used as callback ID) */


/****************************************************************************************/
/*                                                                                      */
/*  End of file                                                                         */
/*                                                                                      */
/****************************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* LVM_TYPES_H */
