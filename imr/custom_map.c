/****************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only
 * intended for use with Renesas products. No other uses are authorized. This
 * software is owned by Renesas Electronics Corporation and is protected under
 * all applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
 * LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
 * TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
 * ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
 * FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
 * ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
 * BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software
 * and to discontinue the availability of this software. By using this software,
 * you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2021 Renesas Electronics Corporation. All rights reserved.
 ****************************************************************************/


#include "include/custom_map.h"
#include "common.h"
#include <math.h>
#include <stdio.h>

unsigned int CustomMapLDC(void * usrData, float x, float y, float * X, float * Y)
{
    undistort_params_t * lensParam = (undistort_params_t *)usrData;

    if (!lensParam) return 1;

    float a       = (x * lensParam->sx - lensParam->cx) / lensParam->fx;
    float b       = (y * lensParam->sy - lensParam->cy) / lensParam->fy;
    float theta   = (float)atan(sqrt(a * a + b * b));
    float theta2  = theta * theta;
    float theta4  = theta2 * theta2;
    float theta6  = theta4 * theta2;
    float divider = (float)sqrt(a * a + b * b);

    if (divider == 0.0) divider = 1.0;

    float temp0  = theta * (1 + lensParam->k1 * theta2 + lensParam->k2 * theta4 + lensParam->k3 * theta6);
    float corr_x = (a * temp0) / divider * lensParam->fx + lensParam->cx;
    float corr_y = (b * temp0) / divider * lensParam->fy + lensParam->cy;
    *X           = corr_x;
    *Y           = corr_y;
    // *X           = corr_x*((float)g_frame_width/IMR_Resize_Width);
    // *Y           = corr_y*((float)g_frame_height/IMR_Resize_Height);

    return 0;
}

unsigned int CustomMapResize(void * usrData, float x, float y, float * X, float * Y)
{
    map_params_t * map_params = (map_params_t *)usrData;

    *X = map_params->w1 * x + 0;
    *Y = map_params->w2 * y + 0;

    // *X = map_params->w1 * x - map_params->w2 * y+1280;
    // *Y = map_params->w1 * y + map_params->w2 * x+800;
    // float rotatedX = (float)(x*cos(map_params->w1) - y*sin(map_params->w2));
    // float rotatedY = (float)(x*sin(map_params->w1) + y*cos(map_params->w2));
    // *X = rotatedX + 440;
    // *Y = rotatedY + 800;

    // *X = (float)(x*cos(90) - y*sin(90));
    // *Y = (float)(x*sin(90) + y*cos(90));

    return 0;
}
