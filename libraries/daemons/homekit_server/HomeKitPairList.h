/**
 ******************************************************************************
 * @file    HomeKitPairList.h
 * @author  William Xu
 * @version V1.0.0
 * @date    05-Oct-2014
 * @brief
 ******************************************************************************
 *  UNPUBLISHED PROPRIETARY SOURCE CODE
 *  Copyright (c) 2016 MXCHIP Inc.
 *
 *  The contents of this file may not be disclosed to third parties, copied or
 *  duplicated in any form, in whole or in part, without the prior written
 *  permission of MXCHIP Corporation.
 ******************************************************************************
 */

#pragma once

#include "common.h"
#include "platform.h"
#include "platform_config.h"

#define MaxControllerNameLen  64
#define MaxPairRecord         16

/*Pair Info flash content*/
typedef struct _pair_t {
  char             controllerName[MaxControllerNameLen];
  uint8_t          controllerLTPK[32];
  int              permission;
} _pair_t;

typedef struct _pair_list_in_flash_t {
  _pair_t          pairInfo[MaxPairRecord];
} pair_list_in_flash_t;

/* Malloc a memory and */
//merr_t HKReadPairList(pair_list_in_flash_t **pPairList);

uint32_t HKPairInfoCount(void);

merr_t HKPairInfoClear(void);

merr_t HKPairInfoInsert(char controllerIdentifier[64], uint8_t controllerLTPK[32], bool admin);

merr_t HKPairInfoFindByName(char controllerIdentifier[64], uint8_t foundControllerLTPK[32], bool *isAdmin );

merr_t HKPairInfoFindByIndex(uint32_t index, char controllerIdentifier[64], uint8_t foundControllerLTPK[32], bool *isAdmin );

merr_t HKPairInfoRemove(char controllerIdentifier[64]);


