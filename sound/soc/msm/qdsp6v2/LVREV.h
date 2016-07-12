/************************************************************************/
/*  Copyright (c) 2004-2007 NXP Software. All rights are reserved.      */
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

/****************************************************************************************/
/*                                                                                      */
/*     Project::                                                                        */
/*     $Author: nxp27078 $*/
/*     $Revision: 15047 $*/
/*     $Date: 2011-06-10 02:18:44 +0900 (±Ý, 10 6 2011) $*/
/*                                                                                      */
/****************************************************************************************/

/****************************************************************************************/
/*                                                                                      */
/*  Header file for the application layer interface of the LVREV module                 */
/*                                                                                      */
/*  This files includes all definitions, types, structures and function prototypes      */
/*  required by the calling layer. All other types, structures and functions are        */
/*  private.                                                                            */
/*                                                                                      */
/****************************************************************************************/

#ifndef __LVREV_H__
#define __LVREV_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/****************************************************************************************/
/*                                                                                      */
/*  Includes                                                                            */
/*                                                                                      */
/****************************************************************************************/
#include "LVC_Types.h"


/****************************************************************************************/
/*                                                                                      */
/*  Definitions                                                                         */
/*                                                                                      */
/****************************************************************************************/
/* General */
#define LVREV_BLOCKSIZE_MULTIPLE                1       /* Processing block size multiple */
#define LVREV_MAX_T60                        7000       /* Maximum decay time is 7000ms */

/* Memory table*/
#define LVREV_NR_MEMORY_REGIONS                 4       /* Number of memory regions */


/****************************************************************************************/
/*                                                                                      */
/*  Types                                                                               */
/*                                                                                      */
/****************************************************************************************/
/* Instance handle */
typedef void *LVREV_Handle_t;


/* Status return values */
typedef enum
{
    LVREV_SUCCESS            = 0,                       /* Successful return from a routine */
    LVREV_ALIGNMENTERROR     = 1,                       /* Memory alignment error */
    LVREV_NULLADDRESS        = 2,                       /* NULL allocation address */
    LVREV_OUTOFRANGE         = 3,                       /* Out of range control parameter */
    LVREV_INVALIDNUMSAMPLES  = 4,                       /* Invalid number of samples */
    LVREV_RETURNSTATUS_DUMMY = LVM_MAXENUM
} LVREV_ReturnStatus_en;


/* Reverb delay lines */
typedef enum
{
    LVREV_DELAYLINES_1     = 1,                         /* One delay line */
    LVREV_DELAYLINES_2     = 2,                         /* Two delay lines */
    LVREV_DELAYLINES_4     = 4,                         /* Four delay lines */
    LVREV_DELAYLINES_DUMMY = LVM_MAXENUM
} LVREV_NumDelayLines_en;


/****************************************************************************************/
/*                                                                                      */
/*  Structures                                                                          */
/*                                                                                      */
/****************************************************************************************/

/* Memory table containing the region definitions */
typedef struct
{
    LVM_MemoryRegion_st        Region[LVREV_NR_MEMORY_REGIONS];  /* One definition for each region */
} LVREV_MemoryTable_st;


/* Control Parameter structure */
typedef struct
{
    /* General parameters */
    LVM_Mode_en                 OperatingMode;          /* Operating mode */
    LVM_Fs_en                   SampleRate;             /* Sample rate */
    LVM_Format_en               InputSourceFormat;      /* Input Source data format */
    LVM_Format_en               OutputSourceFormat;     /* Output Source data format */
    LVM_Mode_en                 DryWetMixEnable;        /* Dry and Wet Signal mixing enable or disable */

    /* Parameters for REV */
    LVM_UINT16                  Level;                  /* Level, 0 to 100 representing percentage of reverb */
    LVM_UINT16                  LPF;                    /* Low pass filter, in Hz */
    LVM_UINT16                  HPF;                    /* High pass filter, in Hz */
    LVM_UINT16                  T60;                    /* Decay time constant, in ms */
    LVM_UINT16                  Density;                /* Echo density, 0 to 100 for minimum to maximum density */
    LVM_UINT16                  Damping;                /* Damping */
    LVM_UINT16                  RoomSize;               /* Simulated room size, 1 to 100 for minimum to maximum size */

} LVREV_ControlParams_st;


/* Instance Parameter structure */
typedef struct
{
    /* General */
    LVM_UINT16                  MaxBlockSize;           /* Maximum processing block size */

    /* Reverb */
    LVREV_NumDelayLines_en      NumDelays;              /* The number of delay lines, 1, 2 or 4 */

} LVREV_InstanceParams_st;


/****************************************************************************************/
/*                                                                                      */
/*  Function Prototypes                                                                 */
/*                                                                                      */
/****************************************************************************************/

/****************************************************************************************/
/*                                                                                      */
/* FUNCTION:                LVREV_GetMemoryTable                                        */
/*                                                                                      */
/* DESCRIPTION:                                                                         */
/*  This function is used to obtain the LVREV module memory requirements to support     */
/*  memory allocation. It can also be used to return the memory base address provided   */
/*  during memory allocation to support freeing of memory when the LVREV module is no   */
/*  longer required. It is called in two ways:                                          */
/*                                                                                      */
/*  hInstance = NULL                Returns the memory requirements                     */
/*  hInstance = Instance handle     Returns the memory requirements and allocated       */
/*                                  base addresses.                                     */
/*                                                                                      */
/*  When this function is called with hInstance = NULL the memory base address pointers */
/*  will be NULL on return.                                                             */
/*                                                                                      */
/*  When the function is called for freeing memory, hInstance = Instance Handle the     */
/*  memory table returns the allocated memory and base addresses used during            */
/*  initialisation.                                                                     */
/*                                                                                      */
/* PARAMETERS:                                                                          */
/*  hInstance               Instance Handle                                             */
/*  pMemoryTable            Pointer to an empty memory table                            */
/*  pInstanceParams         Pointer to the instance parameters                          */
/*                                                                                      */
/* RETURNS:                                                                             */
/*  LVREV_SUCCESS           Succeeded                                                   */
/*  LVREV_NULLADDRESS       When pMemoryTable is NULL                                   */
/*  LVREV_NULLADDRESS       When requesting memory requirements and pInstanceParams     */
/*                          is NULL                                                     */
/*                                                                                      */
/* NOTES:                                                                               */
/*  1.  This function may be interrupted by the LVREV_Process function                  */
/*                                                                                      */
/****************************************************************************************/
LVREV_ReturnStatus_en LVREV_GetMemoryTable(LVREV_Handle_t           hInstance,
                                           LVREV_MemoryTable_st     *pMemoryTable,
                                           LVREV_InstanceParams_st  *pInstanceParams);


/****************************************************************************************/
/*                                                                                      */
/* FUNCTION:                LVREV_GetInstanceHandle                                     */
/*                                                                                      */
/* DESCRIPTION:                                                                         */
/*  This function is used to create a LVREV module instance. It returns the created     */
/*  instance handle through phInstance. All parameters are set to invalid values, the   */
/*  LVREV_SetControlParameters function must be called with a set of valid control      */
/*  parameters before the LVREV_Process function can be called.                         */
/*                                                                                      */
/*  The memory allocation must be provided by the application by filling in the memory  */
/*  region base addresses in the memory table before calling this function.             */
/*                                                                                      */
/* PARAMETERS:                                                                          */
/*  phInstance              Pointer to the instance handle                              */
/*  pMemoryTable            Pointer to the memory definition table                      */
/*  pInstanceParams         Pointer to the instance parameters                          */
/*                                                                                      */
/* RETURNS:                                                                             */
/*  LVREV_SUCCESS           Succeeded                                                   */
/*  LVREV_NULLADDRESS       When phInstance or pMemoryTable or pInstanceParams is NULL  */
/*  LVREV_NULLADDRESS       When one of the memory regions has a NULL pointer           */
/*                                                                                      */
/* NOTES:                                                                               */
/*                                                                                      */
/****************************************************************************************/
LVREV_ReturnStatus_en LVREV_GetInstanceHandle(LVREV_Handle_t            *phInstance,
                                              LVREV_MemoryTable_st      *pMemoryTable,
                                              LVREV_InstanceParams_st   *pInstanceParams);


/****************************************************************************************/
/*                                                                                      */
/* FUNCTION:                LVXX_GetControlParameters                                   */
/*                                                                                      */
/* DESCRIPTION:                                                                         */
/*  Request the LVREV module control parameters. The current parameter set is returned  */
/*  via the parameter pointer.                                                          */
/*                                                                                      */
/* PARAMETERS:                                                                          */
/*  hInstance               Instance handle                                             */
/*  pControlParams          Pointer to an empty parameter structure                     */
/*                                                                                      */
/* RETURNS:                                                                             */
/*  LVREV_SUCCESS           Succeeded                                                   */
/*  LVREV_NULLADDRESS       When hInstance or pControlParams is NULL                    */
/*                                                                                      */
/* NOTES:                                                                               */
/*  1.  This function may be interrupted by the LVREV_Process function                  */
/*                                                                                      */
/****************************************************************************************/
LVREV_ReturnStatus_en LVREV_GetControlParameters(LVREV_Handle_t           hInstance,
                                                 LVREV_ControlParams_st   *pControlParams);


/****************************************************************************************/
/*                                                                                      */
/* FUNCTION:                LVREV_SetControlParameters                                  */
/*                                                                                      */
/* DESCRIPTION:                                                                         */
/*  Sets or changes the LVREV module parameters.                                        */
/*                                                                                      */
/* PARAMETERS:                                                                          */
/*  hInstance               Instance handle                                             */
/*  pNewParams              Pointer to a parameter structure                            */
/*                                                                                      */
/* RETURNS:                                                                             */
/*  LVREV_SUCCESS           Succeeded                                                   */
/*  LVREV_NULLADDRESS       When hInstance or pNewParams is NULL                        */
/*                                                                                      */
/* NOTES:                                                                               */
/*  1.  This function may be interrupted by the LVREV_Process function                  */
/*                                                                                      */
/****************************************************************************************/
LVREV_ReturnStatus_en LVREV_SetControlParameters(LVREV_Handle_t           hInstance,
                                                 LVREV_ControlParams_st   *pNewParams);


/****************************************************************************************/
/*                                                                                      */
/* FUNCTION:                LVREV_ClearAudioBuffers                                     */
/*                                                                                      */
/* DESCRIPTION:                                                                         */
/*  This function is used to clear the internal audio buffers of the module.            */
/*                                                                                      */
/* PARAMETERS:                                                                          */
/*  hInstance               Instance handle                                             */
/*                                                                                      */
/* RETURNS:                                                                             */
/*  LVREV_SUCCESS          Buffers Cleared                                              */
/*  LVREV_NULLADDRESS      Instance is NULL                                             */
/*                                                                                      */
/* NOTES:                                                                               */
/*                                                                                      */
/****************************************************************************************/
LVREV_ReturnStatus_en LVREV_ClearAudioBuffers(LVREV_Handle_t  hInstance);


/****************************************************************************************/
/*                                                                                      */
/* FUNCTION:                LVREV_Process                                               */
/*                                                                                      */
/* DESCRIPTION:                                                                         */
/*  Process function for the LVREV module.                                              */
/*                                                                                      */
/* PARAMETERS:                                                                          */
/*  hInstance               Instance handle                                             */
/*  pInData                 Pointer to the input data                                   */
/*  pOutData                Pointer to the output data                                  */
/*  NumSamples              Number of samples in the input buffer                       */
/*                                                                                      */
/* RETURNS:                                                                             */
/*  LVREV_SUCCESS           Succeeded                                                   */
/*  LVREV_INVALIDNUMSAMPLES NumSamples was larger than the maximum block size           */
/*  LVREV_ALIGNMENTERROR    When either the input our output buffers are not 32-bit     */
/*                          aligned                                                     */
/*                                                                                      */
/* NOTES:                                                                               */
/*  1. The input and output buffers must be 32-bit aligned                              */
/*                                                                                      */
/****************************************************************************************/
LVREV_ReturnStatus_en LVREV_Process(LVREV_Handle_t      hInstance,
                                    const LVM_INT32     *pInData,
                                    LVM_INT32           *pOutData,
                                    const LVM_UINT16          NumSamples);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif      /* __LVREV_H__ */

/* End of file */
