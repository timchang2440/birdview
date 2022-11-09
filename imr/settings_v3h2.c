/****************************************************************************
 * Copyright [2020-2021] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
 *
 *
 * FILE          : settings_v3h2.c
 * DESCRIPTION   : This module is the sample program to use IMR-LX4 driver.
 * CREATED       : 2021.06.14
 * MODIFIED      : -
 * AUTHOR        : Renesas Electronics Corporation
 * TARGET DEVICE : R-CarV3H_2
 * TARGET OS     : NonOS
 * HISTORY       :
 *                  2021.06.14
 *                - Created new.
 ****************************************************************************/

/*******************************************************************************
*   Title: Device Dependent setting
*
*   This module contains the device dependent setting.
*/

#include "rcar-xos/imr/r_imrdrv_api.h"
#include "rcar-xos/osal/r_osal.h"

#include "include/imrlxsample_syn.h"

#define IMR_RESOURCE_ID 0x1000

osal_mutex_id_t      callback_mutex_id[MAX_MODULE] = {IMR_RESOURCE_ID + 6, IMR_RESOURCE_ID + 7, IMR_RESOURCE_ID + 8, IMR_RESOURCE_ID + 9, IMR_RESOURCE_ID + 10};
osal_cond_id_t       cond_id[MAX_MODULE]           = {IMR_RESOURCE_ID, IMR_RESOURCE_ID + 1, IMR_RESOURCE_ID + 2, IMR_RESOURCE_ID + 3, IMR_RESOURCE_ID + 4};
osal_thread_id_t     thread_id[MAX_MODULE]         = {IMR_RESOURCE_ID, IMR_RESOURCE_ID + 1, IMR_RESOURCE_ID + 2, IMR_RESOURCE_ID + 3, IMR_RESOURCE_ID + 4};
char *                device_name[MAX_MODULE]      = {"ims_00", "ims_01", "ims_02", "imr_00", "imr_01"};
osal_mutex_id_t      mutex_id[MAX_MODULE]          = {IMR_RESOURCE_ID, IMR_RESOURCE_ID + 1, IMR_RESOURCE_ID + 2, IMR_RESOURCE_ID + 3, IMR_RESOURCE_ID + 4};
e_imrdrv_channelno_t channel_no[MAX_MODULE]        = {IMRDRV_CHANNELNO_0, IMRDRV_CHANNELNO_1, IMRDRV_CHANNELNO_2, IMRDRV_CHANNELNO_4, IMRDRV_CHANNELNO_5};

/***************************************************************************
*   Function: convert_channel_to_index
*
*   Description:
*       Convert a channel to an index.
*
*   Parameters:
*       channel_no - [i]Channel number.
*
*   Return:
*       Index
*/
uint32_t convert_channel_to_index(e_imrdrv_channelno_t channel_no)
{
    uint32_t ret;

    switch (channel_no)

    {
        case IMRDRV_CHANNELNO_0:
            ret = 0U;
            break;
        case IMRDRV_CHANNELNO_1:
            ret = 1U;
            break;
        case IMRDRV_CHANNELNO_2:
            ret = 2U;
            break;
        case IMRDRV_CHANNELNO_4:
            ret = 3U;
            break;
        case IMRDRV_CHANNELNO_5:
            ret = 4U;
            break;
        default:
            ret = MAX_MODULE;
            break;
    }

    return ret;
}
