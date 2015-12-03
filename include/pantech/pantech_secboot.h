#ifndef BOOT_PANTECH_SECBOOT_TYPE_H
#define BOOT_PANTECH_SECBOOT_TYPE_H

/*=============================================================================

                              Boot Loader

GENERAL DESCRIPTION
  This module performs binary image and AMSS loading.

Copyright 2010-2012 by QUALCOMM Technologies, Incorporated.  All Rights Reserved.
=============================================================================*/

/*===========================================================================

                           EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

$Header: //source/qcom/qct/core/pkg/bootloaders/rel/2.0/boot_images/core/boot/secboot3/src/boot_loader.h#6 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
02/01/13   lsi      create
============================================================================*/

/*===========================================================================

                           INCLUDE FILES
						   
===========================================================================*/


/*=============================================================================

                              DEFINITIONS

=============================================================================*/
#if defined (F_PANTECH_SECBOOT) || defined (CONFIG_PANTECH_SECBOOT)

/* 20140312-LS1-JHM modified : add the validation check flag for secure boot */
#define SECBOOT_BIN_MSM_ID_ERR 0x01


typedef struct
{ 
  #define SECBOOT_FUSE_FLAG_MAGIC_NUM     0xAAFFFF
  #define SECBOOT_FUSE_FLAG_UNSET         0xF0F0F0
  #define SECBOOT_FUSE_FLAG_SET           0xF1F1F1
  #define SECBOOT_FUSE_NOT_BLOWN          0xF2F2F2
  #define SECBOOT_FUSE_BLOWN              0xF3F3F3  

 #if defined(__KERNEL__)
  uint32_t secboot_magic_num;

  uint32_t auth_en;
  uint32_t shk_blow;
  uint32_t shk_rw_dis;
  uint32_t jtag_dis;
  /* LS1-JHM modified : add the model ID for secure boot */
  uint32_t model_id;
  /* 20140312-LS1-JHM modified : add the validation check flag for secure boot */
  uint32_t msm_id_bld;
  uint32_t msm_id_dev;
 #else //#elif defined(_UINT32_DEFINED)
  uint32 secboot_magic_num;

  uint32 auth_en;
  uint32 shk_blow;
  uint32 shk_rw_dis;
  uint32 jtag_dis;
  /* LS1-JHM modified : add the model ID for secure boot */
  uint32 model_id;
  /* 20140312-LS1-JHM modified : add the validation check flag for secure boot */
  uint32 msm_id_bld;
  uint32 msm_id_dev;
 #endif

} secboot_fuse_flag;

typedef struct
{ 
  #define SECBOOT_SECBOOT_CHECK_MAGIC_NUM     0xCCBBBB
  #define SECBOOT_AUTH_IMG_NUM                16

 #if defined(__KERNEL__)
  uint32_t secboot_magic_num;
  uint32_t auth_img_result[SECBOOT_AUTH_IMG_NUM];  
 #else  //#elif defined(_UINT32_DEFINED)
  uint32 secboot_magic_num;
  uint32 auth_img_result[SECBOOT_AUTH_IMG_NUM];
 #endif

} secboot_auth_img_result;
#endif  //F_PANTECH_SECBOOT

/*===========================================================================

                      PUBLIC FUNCTION DECLARATIONS

===========================================================================*/


/*===========================================================================
**  Function :  boot_load_image_header
** ==========================================================================
*/

#endif /* BOOT_PANTECH_SECBOOT_TYPE_H */

