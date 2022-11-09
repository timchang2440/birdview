/***********************************************************************************************************************
* File Name    : format.c
* Version      : 1.0.0
* Product Name : DMS Application
* Device(s)    : R-Car V3h2
* Description  : Video Output format File
***********************************************************************************************************************/

/***********************************************************************************************************************
* History : Version DD.MM.YYYY Description
*         : 1.0.0   20.05.2022 Initial version 
***********************************************************************************************************************/

/* System includes */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#if 0
#include <drm_fourcc.h>
#else
#include <libdrm/drm_fourcc.h>
#endif

/*util library */
#include "include/util/format.h"
#include "common.h"

/**
 * Configuration by defines
 */
#define MAKE_RGB_INFO(rl, ro, gl, go, bl, bo, al, ao) \
    .rgb = { { (rl), (ro) }, { (gl), (go) }, { (bl), (bo) }, { (al), (ao) } }
#define MAKE_YUV_INFO(order, xsub, ysub, chroma_stride) \
    .yuv = { (order), (xsub), (ysub), (chroma_stride) }
#define VAL_ZERO            (0)


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

static const struct util_format_info format_info[] = {
    /* YUV packed */
    {DRM_FORMAT_UYVY, "UYVY", MAKE_YUV_INFO(YUV_YCbCr | YUV_CY, 2, 2, 2)},
    {DRM_FORMAT_VYUY, "VYUY", MAKE_YUV_INFO(YUV_YCrCb | YUV_CY, 2, 2, 2)},
    {DRM_FORMAT_YUYV, "YUYV", MAKE_YUV_INFO(YUV_YCbCr | YUV_YC, 2, 2, 2)},
    {DRM_FORMAT_YVYU, "YVYU", MAKE_YUV_INFO(YUV_YCrCb | YUV_YC, 2, 2, 2)},
    /* RGB24 */
    { DRM_FORMAT_BGR888, "BG24", MAKE_RGB_INFO(8, 0, 8, 8, 8, 16, 0, 0) },
    { DRM_FORMAT_RGB888, "RG24", MAKE_RGB_INFO(8, 16, 8, 8, 8, 0, 0, 0) },
};

uint32_t util_format_fourcc(const char * name)
{
    unsigned int i = VAL_ZERO;

    for (i = 0; i < ARRAY_SIZE(format_info); i++)

        if (!strcmp(format_info[i].name, name))
            return format_info[i].format;

    return SUCCESS;
}

const struct util_format_info * util_format_info_find(uint32_t format)
{
    unsigned int i = VAL_ZERO;

    for (i = 0; i < ARRAY_SIZE(format_info); i++)

        if (format_info[i].format == format)
            return &format_info[i];

    return NULL;
}
