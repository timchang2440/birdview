/****************************************************************************
 * Copyright [2020-2021] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
 *
 *
 * FILE          : imrlxsample_syn.h
 * DESCRIPTION   : This module is the sample program to use IMR-LX4 driver.
 * CREATED       : 2021.10.01
 * MODIFIED      : -
 * AUTHOR        : Renesas Electronics Corporation
 * TARGET DEVICE : R-CarV3U/V4H
 * TARGET OS     : NonOS
 * HISTORY       :
 *                 2021.10.01
 *                 - Created new.
 *
 ****************************************************************************/

/*******************************************************************************
*   Title: IMR device driver sample main
*
*   This module contains the IMR device driver sample main.
*   It must be included in any C Module using those functions
*/


#ifndef IMRLXSAMPLE_H_
#define IMRLXSAMPLE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "common.h"

/***************************************************************************
*   Section: Defines
*/

/* Width and height of source and destination pictures. */

#define MAX_MODULE_NUM (5)


/***************************************************************************
*   Section: Global Variables
*/

extern osal_mutex_id_t       mutex_id[MAX_MODULE];
extern osal_mutex_id_t       callback_mutex_id[MAX_MODULE];
extern osal_cond_id_t        cond_id[MAX_MODULE];
extern osal_thread_id_t      thread_id[MAX_MODULE];
extern char *                device_name[MAX_MODULE];
typedef uint64_t             addr_t;
extern e_imrdrv_channelno_t  channel_no[MAX_MODULE];

/***************************************************************************
*   Section: Global Functions
*/

/***************************************************************************
*   Function: get_display_list_addr
*
*   Description:
*       Get global variable g_dl2_top_dst_yuvsep.
*
*   Parameters:
*       p_arraynum - [o]Length of g_dl2_top_dst_yuvsep[] array.
*
*   Return:
*       Global variable g_dl2_top_dst_yuvsep.
*/
extern const uint32_t * get_display_list_addr(uint32_t * const p_arraynum);

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
extern uint32_t convert_channel_to_index(e_imrdrv_channelno_t channel_no);

#ifdef __cplusplus
}
#endif


#endif

/* End of File */

