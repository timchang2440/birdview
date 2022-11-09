/******************************************************************************
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
 ******************************************************************************/

#ifndef HELPER_CUSTOM_MAP_H_
#define HELPER_CUSTOM_MAP_H_

#include <stdint.h>

/**
 * Parameters for undistortion
 */
typedef struct undistort_params {
    float k1,k2,k3; // radial distortion parameters (last one optional)
    float p1,p2;    // tangential distortion parameters
    float fx,fy;    // focal length ratio
    float cx,cy;    // optical center
    float sx,sy;    // prescaling factors (source.width/dest.width and height)
} undistort_params_t;

typedef struct map_params {
    float w1, w2;    // maping coefficients
} map_params_t;

unsigned int CustomMapLDC(void * usrData, float x, float y, float * X, float * Y);
unsigned int CustomMapResize(void * usrData, float x, float y, float * X, float * Y);
#endif /* HELPER_CUSTOM_MAP_H_ */
