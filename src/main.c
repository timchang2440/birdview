/***********************************************************************************************************************
*
* This software is Copyright (C) 2018,2020-2021 Renesas Electronics Corporation
*
* You may use, distribute and copy this software under the terms of the MIT
* license displayed below.
*
* -----------------------------------------------------------------------------
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
*  Alternatively, the contents of this file may be used under the terms of
*  the GNU General Public License Version 2 ("GPL") in which case the provisions
*  of GPL are applicable instead of those above.
*
*  If you wish to allow use of your version of this file only under the terms of
*  GPL, and not to allow others to use your version of this file under the terms
*  of the MIT license, indicate your decision by deleting the provisions above
*  and replace them with the notice and other provisions required by GPL as set
*  out in the file called "GPL-COPYING" included in this distribution. If you do
*  not delete the provisions above, a recipient may use your version of this file
*  under the terms of either the MIT license or GPL.
*
* -----------------------------------------------------------------------------
*
*  EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
*  PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
*  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
*  PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
*  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
*  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
*  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
* -----------------------------------------------------------------------------
***********************************************************************************************************************/
/****************************************************************************
 * FILE          : main.c
 * DESCRIPTION   : This module is the sample program to use IMR-LX4 driver.
 * CREATED       : 2018.06.15
 * MODIFIED      :
 * AUTHOR        : Renesas Electronics Corporation
 * TARGET DEVICE : R-Car Gen3e
 * HISTORY       :
 *                 2018.06.15
 *                 - Created new.
 *                 2020.05.18
 *                 -Adapted to OSAL API Rev.0.73.
 *                 2020.12.11
 *                 -Adapted to xOS2
 *                 2021.2.10
 *                - Fix compile warning
 *                 2021.4.14
 *                - Change parameter for R_OSAL_MmngrOpen
 *                - Handling return value
 *                 2021.5.6
 *                - IMR Channel execute parallel
 *                 2021.5.12
 *                - Sample app support multi target device
 *                 2021.6.14
 *                - Adapted to IMR Driver Ver.2.2.0
 *                - Remove unnecessary macro definition
 *                - Modify macro definition value
 *                - Modify check item for checking destination data in execute_imrdrv()
 *                - Modify callback_func() to check details_code.
 *
 ****************************************************************************/


/*******************************************************************************
*   Title: IMR device driver sample main
*
*   This module contains the IMR device driver sample main.
*/


/***************************************************************************
*   Section: Includes
*/
#include <fcntl.h>    /* For O_RDWR */
#include <unistd.h>   /* For open(), creat() */
#include <termios.h>
#include <malloc.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "rcar-xos/imr/r_imrdrv_api.h"
#include "rcar-xos/osal/r_osal.h"
#include "include/vin_capture.h"
#include "vout_main.h"
#include "common.h"
#include "customize.h"

/***************************************************************************
*   Section: Defines
*/

/** Unchangeable definitions of sample application **/
#define DATA_SIZE_ALIGN (256U) /* Alignment of data size */
#define DL_SIZE_ALIGN   (64U)  /* Alignment of DL size */
#define TIMEOUT         (300000U) /* msec */

#define OSAL_MUTEX_WAIT (10000U) /* msec */

#define IMR_SIZEOF_DL_OPERAND (4U) /* Size fo IMR DL Operand */
#define IMR_DL_SIZE(d)  ((d) * IMR_SIZEOF_DL_OPERAND)



/** Changeable definitions of sample application **/
#define RENDERING_LOOP_NUM   (3) /* The number of IMR module for execute rendering */

#if 1 /* Enable when debugging the rendering process. */
#define IMRSMP_DEBUG_PRINTF(fmt, ...)  printf(fmt, __VA_ARGS__);
#else
#define IMRSMP_DEBUG_PRINTF(fmt, ...)
#endif

/***************************************************************************
*   Section: Local Functions Prototypes
*/

/***************************************************************************
*   Section: Global Functions
*/
bool is_thread_exit             = false;
unsigned char * vin_out_buffer  = NULL;
unsigned char * imr_ldc_buffer  = NULL;
unsigned char * imr_ldc_in      = NULL;
unsigned char * imr_res_in      = NULL;
unsigned char * vout_in         = NULL;
osal_mutex_handle_t mtx_handle  = OSAL_MUTEX_HANDLE_INVALID;

unsigned long StartTime;
unsigned long FinishTime;
unsigned long Diff;
static int    Fps = 0;

systemstatus dmsStatus;

#define ENABLE_IMR      1

unsigned long GetTimer()
{
    long            retval;
    struct timeval  Time;
    struct timezone TimeZone;

    gettimeofday(&Time, &TimeZone);

    retval = Time.tv_sec * 1000 + Time.tv_usec / 1000;

    return ((long unsigned int)retval);
}

int64_t R_Capture_Task(void* s)
{
    int ret = INVALID;

    while(!is_thread_exit)
    {
        {
            ret = R_VIN_Execute();

            if (FAILED == ret)
            {
                PRINT_ERROR("Failed R_VIN_Execute \n");
                is_thread_exit = true;
                return FAILED;
            }
        }
        R_OSAL_ThreadSleepForTimePeriod ((osal_milli_sec_t)TIMEOUT_5MS_SLEEP);
    }
    return SUCCESS;
}

int64_t R_IMR_Task(void* s)
{
    int ret;

    R_OSAL_ThreadSleepForTimePeriod ((osal_milli_sec_t)TIMEOUT_50MS_SLEEP);
    // StartTime = GetTimer();
    while(!is_thread_exit)
    {
        {
            ret = R_IMR_ExecuteLDC();
            if (FAILED == ret)
            {
                PRINT_ERROR("Failed R_IMR_ExecuteLDC \n");
                is_thread_exit = true;
                return FAILED;
            }
          #if 1
            FinishTime = GetTimer();
            Diff       = (FinishTime - StartTime);

            Fps++;
            //time_total += (unsigned int)Diff;
            //DEBUG_PRINT("time_total: %u\n", time_total);

            if (Diff / 1000 >= 1)
            {
                printf("DEBUG: %s(%d): <%d fps>\n", __func__, __LINE__, Fps);
                //DEBUG_PRINT("<%dfps>\n", Fps);
                Fps        = 0;
                StartTime = GetTimer();
            }
          #endif
        }
      #if 0
        ret = R_IMR_ExecuteResize();
        if (FAILED == ret)
        {
            PRINT_ERROR("Failed R_IMR_ExecuteResize \n");
            is_thread_exit = true;
            return FAILED;
        }
      #endif

        R_OSAL_ThreadSleepForTimePeriod ((osal_milli_sec_t)TIMEOUT_5MS_SLEEP);

    }
    return SUCCESS;
}

int64_t R_VOUT_Task(void* s)
{
    int ret = INVALID;

    R_OSAL_ThreadSleepForTimePeriod ((osal_milli_sec_t)TIMEOUT_50MS_SLEEP);

    while(!is_thread_exit)
    {
      /*
        ret = f_opencv_execute(driverActivity, det_accuracy);
        if (FAILED == ret)
        {
            is_thread_exit = true;
            PRINT_ERROR("Failed f_opencv_execute \n");
            return FAILED;
        }
      */

        ret = execute();
        if (FAILED == ret)
        {
            is_thread_exit = true;
            PRINT_ERROR("Failed Vout execute \n");
            return FAILED;
        }

        R_OSAL_ThreadSleepForTimePeriod ((osal_milli_sec_t)TIMEOUT_10MS_SLEEP);

    }

    return SUCCESS;

}

#define VIN_Device_0  0
#define VIN_Device_1  1
int64_t R_Init_Modules()
{
    int ret = INVALID;
    char csi_ch[64];
    char media_cmd[256];

    strcpy(csi_ch, "feaa0000.csi_00");
    //strcpy(csi_ch, "feab0000.csi_01");

    ret = system("media-ctl -d /dev/media0 -r");
    for (int i=0 ; i<1 ; i++) {
        sprintf(media_cmd, "media-ctl -d /dev/media0 -l \"'rcar_csi2 %s':%d -> 'VIN%d output':0 [1]\"", csi_ch, i+1, i);
        ret = system(media_cmd);
        sprintf(media_cmd, "media-ctl -d /dev/media0 -V \"'rcar_csi2 %s':%d [fmt:UYVY8_2X8/1280x800 field:none]\"", csi_ch, i+1);
        ret = system(media_cmd);
    }
    
    /* Initialize all modules */
    ret = R_VIN_Initialize();
    if (FAILED == ret)
    {
        PRINT_ERROR("Failed R_VIN_Initilize \n");
        return FAILED;
    }
#if ENABLE_IMR
    ret = R_IMR_Init();
    if (FAILED == ret)
    {
        PRINT_ERROR("Failed R_IMR_Init \n");
        return FAILED;
    }
    
    #ifdef NV16 //common.h
    {
        ret = R_IMR_SetupLDC(9);
        if (FAILED == ret)
        {
            PRINT_ERROR("Failed R_IMR_SetupLDC \n");
            return FAILED;
        }
        ret = R_IMR_SetupLDC(10);
        if (FAILED == ret)
        {
            PRINT_ERROR("Failed R_IMR_SetupLDC \n");
            return FAILED;
        }
    }
    #else
    {
        ret = R_IMR_SetupLDC();
        if (FAILED == ret)
        {
            PRINT_ERROR("Failed R_IMR_SetupLDC \n");
            return FAILED;
        }
    }
    #endif
#endif  // ENABLE_IMR
/*
    ret = R_LDE_Init();
    if (FAILED == ret)
    {
        PRINT_ERROR("Failed R_LDE_Init \n");
        return FAILED;
    }
*/
    ret = vout_init();
    if (FAILED == ret)
    {
        PRINT_ERROR("Failed vout_init \n");
        return FAILED;
    }
    return SUCCESS;
}
/***************************************************************************
*   Function: main
*/
int main(void)
{
  e_osal_return_t     osal_ret, ret_osal;
  int ret;
  vin_out_buffer = (unsigned char *) malloc(g_frame_width * g_frame_height * BPP_YUV422);
  if(NULL == vin_out_buffer)
  {
      DEBUG_PRINT("Failed to allocate cnn_rgb_buffer Buffer \n");
      return FAILED;
  }
  else
    DEBUG_PRINT("vin_out_buffer malloc size %u\n", g_frame_width * g_frame_height * BPP_YUV422);
  
  imr_ldc_buffer = (unsigned char *) malloc(g_frame_width * g_frame_height * BPP_YUV422);
  // imr_ldc_buffer = (unsigned char *) malloc(g_frame_width * g_frame_height * BPP_YUV420);
  if (NULL == imr_ldc_buffer)
  {
      DEBUG_PRINT("Failed to allocate imr_ldc_buffer Buffer \n");
      return FAILED;
  }
  else
    // DEBUG_PRINT("imr_ldc_buffer malloc size %f\n", g_frame_width * g_frame_height * BPP_YUV420);
    DEBUG_PRINT("imr_ldc_buffer malloc size %f\n", g_frame_width * g_frame_height * BPP_YUV422);
  
  imr_ldc_in = vin_out_buffer;
  imr_res_in = imr_ldc_buffer;
  vout_in    = imr_ldc_buffer;

  osal_ret= R_OSAL_Initialize();
  if (OSAL_RETURN_OK != osal_ret)
  {
    PRINT_ERROR("OSAL Initialization failed with error %d\n", osal_ret);
  }
  
#if ENABLE_IMR
  ret = init_mmgr();
  if (ret)
  {
    ret_osal = R_OSAL_Deinitialize();
    if (OSAL_RETURN_OK != ret_osal )
    {
      PRINT_ERROR("Failed R_OSAL_Deinitialize ret=%d\n", ret_osal);
    }
  }
#endif  // ENABLE_IMR

  ret = R_Init_Modules();
  if (FAILED == ret)
  {
      //R_Deinit_Modules();
      PRINT_ERROR("Failed R_Init_Modules\n");
      return FAILED;
  }
  
  /* Create Capture thread */
  osal_thread_handle_t    capture_thrd_hndl         = OSAL_THREAD_HANDLE_INVALID;
  int64_t                 thrd_return_value = -1;
  st_osal_thread_config_t capture_thrd_cfg;
  capture_thrd_cfg.func       = R_Capture_Task;
  capture_thrd_cfg.priority   = OSAL_THREAD_PRIORITY_TYPE1;
  capture_thrd_cfg.stack_size = 0x2000;
  capture_thrd_cfg.task_name  = "R_Capture_Task";
  capture_thrd_cfg.userarg    = NULL;
  
#if ENABLE_IMR
  /* Create IMR thread */
  osal_thread_handle_t    imr_thrd_hndl         = OSAL_THREAD_HANDLE_INVALID;
  int64_t                 imr_thrd_return_value = -1;
  st_osal_thread_config_t imr_thrd_cfg;
  imr_thrd_cfg.func       = R_IMR_Task;
  imr_thrd_cfg.priority   = OSAL_THREAD_PRIORITY_TYPE0;
  imr_thrd_cfg.stack_size = 0x2000;
  imr_thrd_cfg.task_name  = "R_IMR_Task";
  imr_thrd_cfg.userarg    = NULL;
#endif  // ENABLE_IMR

  /* Create Vout thread */
  osal_thread_handle_t    vout_thrd_hndl         = OSAL_THREAD_HANDLE_INVALID;
  int64_t                 vout_thrd_return_value = -1;
  st_osal_thread_config_t vout_thrd_cfg;
  vout_thrd_cfg.func       = R_VOUT_Task;
  vout_thrd_cfg.priority   = OSAL_THREAD_PRIORITY_TYPE0;
  vout_thrd_cfg.stack_size = 0x2000;
  vout_thrd_cfg.task_name  = "R_VOUT_Task";
  vout_thrd_cfg.userarg    = NULL;
  

  /* Start Capture Thread */
  osal_ret = R_OSAL_ThreadCreate(&capture_thrd_cfg, 0xf000, &capture_thrd_hndl);
  if (OSAL_RETURN_OK != osal_ret)
  {
      is_thread_exit=true;
      PRINT_ERROR("OSAL capture thread creation failed with error %d\n", osal_ret);
      //R_Deinit_Modules();
      return 0;
  }
  printf("Capture Thread Success\n");

#if ENABLE_IMR
  /* Start IMR Thread */
  osal_ret = R_OSAL_ThreadCreate(&imr_thrd_cfg, 0xf004, &imr_thrd_hndl);
  if (OSAL_RETURN_OK != osal_ret)
  {
      PRINT_ERROR("OSAL IMR thread creation failed with error %d\n", osal_ret);
      //R_Deinit_Modules();
      return 0;
  }
#endif  // ENABLE_IMR

  /* Start vout Thread */
  osal_ret = R_OSAL_ThreadCreate(&vout_thrd_cfg, 0xf002, &vout_thrd_hndl);
  if (OSAL_RETURN_OK != osal_ret)
  {
    is_thread_exit=true;
    PRINT_ERROR("OSAL vout thread creation failed with error %d\n", osal_ret);
    //R_Deinit_Modules();
    return 0;
  }
  printf("Vout Thread Success\n");


  /* wait until capture thread */
  osal_ret = R_OSAL_ThreadJoin(capture_thrd_hndl, &thrd_return_value);
  if (OSAL_RETURN_OK != osal_ret)
  {
      PRINT_ERROR("OSAL capture thread join failed with error %d\n", osal_ret);
      //R_Deinit_Modules();
      return 0;
  }

#if ENABLE_IMR
  osal_ret = R_OSAL_ThreadJoin(imr_thrd_hndl, &imr_thrd_return_value);
  if (OSAL_RETURN_OK != osal_ret)
  {
      PRINT_ERROR("OSAL imr thread join failed with error %d\n", osal_ret);
      //R_Deinit_Modules();
      return 0;
  }
#endif  // ENABLE_IMR

  /* wait until VOUT thread finished */
  osal_ret = R_OSAL_ThreadJoin(vout_thrd_hndl, &vout_thrd_return_value);
  if (OSAL_RETURN_OK != osal_ret)
  {
    PRINT_ERROR("OSAL vout thread join failed with error %d\n", osal_ret);
    //R_Deinit_Modules();
    return 0;
  }
  
  printf("hello world!!\n");
  return 0;
}

/* End of File */
