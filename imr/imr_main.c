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

/**
 * @package imrlxsample_syn
 *
 * This software package demonstrates how input image texture is
 * rendered using IMR LX4(V3U) or LX5(V4H) by means of the
 * bandwidth-optimizing display list generation library .
 */

/* System includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

/* IMR Driver, OSAL and DLGen library */
#include <rcar-xos/imr/r_imrdrv_api.h>
#include <rcar-xos/imr/r_imrdrv_type.h>
#include <rcar-xos/imr/r_imrdlg.h>
#include <rcar-xos/osal/r_osal.h>
#include "include/imrlxsample_syn.h"
#include "include/reg_settings.h"

/* Local includes */
#include "common.h"
#include "customize.h"
#include "include/custom_map.h"
#include "include/osaldrv_helper.h"                         // abstracts the OSAL/driver init overhead


//#define IMR_SAVE_FILE
#ifdef IMR_SAVE_FILE
FILE *IMR_FD = NULL;
#endif  // IMR_SAVE_FILE

/* Test pattern to use as input */
enum
{
    INPUT_BLACK,
    INPUT_WHITE,
    INPUT_GRAY50,
    INPUT_HBAR,
    INPUT_VBAR,
    INPUT_HVBAR,
    INPUT_HLINES,
    INPUT_VLINES,
    INPUT_HBARSTRIPES,
    INPUT_VBARSTRIPES,
    INPUT_CHECKER,
    INPUT_CHECKER_HVBAR
};

/**
 * Enum for pixel format
 */
typedef enum {
    PIXFMT_Y12 = 0,
    PIXFMT_Y8,
    PIXFMT_UV8,
    PIXFMT_UV8_422,
    PIXFMT_YUYV,
    PIXFMT_UYVY,
    PIXFMT_Y10,
    PIXFMT_Y16,
    PIXFMT_NV12,
    PIXFMT_SEP_Y,
    PIXFMT_SEP_UV,
} pixfmt_e;


/**
 * Configuration by defines
 */
#define MESH_SIZE           (4)
#define MESH_SIZE_LIMIT     (0)
#define IMR_SRC_ALIGN       (64U)
#define IMR_DST_ALIGN       (64U)
#define IMR_DL_ALIGN        (256U)                                                                              // according to driver (real HW differs !)
#define IMR_STRIDE_RS       (224)
#define VAL_ZERO            (0)
#define VAL_ONE             (1)


/**
 * Static Variables
 */
static int    mesh_size                     = MESH_SIZE;
static int    mesh_size_limit               = MESH_SIZE_LIMIT;                                            // use library default
static int    inputWidth                    = INVALID;
static int    inputHeight                   = INVALID;
static int    outputWidth                   = INVALID;
static int    outputHeight                  = INVALID;

static int    inputWidth_Y                  = INVALID;
static int    inputHeight_Y                 = INVALID;
static int    outputWidth_Y                 = INVALID;
static int    outputHeight_Y                = INVALID;
static int    inputWidth_UV                 = INVALID;
static int    inputHeight_UV                = INVALID;
static int    outputWidth_UV                = INVALID;
static int    outputHeight_UV               = INVALID;

static int    enable_prefetch               = 1;
static pixfmt_e pixfmt                      = PIXFMT_YUYV;
static p_imrdlg_mapping_function_t map_func = NULL;
static void * map_func_data                 = NULL;
static undistort_params_t opencv_coeff      = {VAL_ZERO, VAL_ZERO, VAL_ZERO, VAL_ZERO, VAL_ZERO, VAL_ZERO, VAL_ZERO, VAL_ZERO, VAL_ZERO, VAL_ZERO, VAL_ZERO};
static map_params_t map_coeff               = {VAL_ZERO, VAL_ZERO};
static addr_t src_addr_ldc                  = VAL_ZERO;
static addr_t dst_addr_ldc                  = VAL_ZERO;
static addr_t dl_addr_ldc                   = VAL_ZERO;
static addr_t src_addr_scaling              = VAL_ZERO;
static addr_t dst_addr_scaling              = VAL_ZERO;
static addr_t dl_addr_scaling               = VAL_ZERO;
static st_imrdrv_attr_param_t      imr_param_ldc;                                                          // Channel 0 params
static st_imrdrv_attr_param_t      imr_param_scaling;                                                      // Channel 1 params
static osal_memory_buffer_handle_t dl_buffer_scaling;
static osal_memory_buffer_handle_t dl_buffer_ldc;
//add Here

static addr_t src_addr_ldc_y                = VAL_ZERO;
static addr_t src_addr_ldc_uv               = VAL_ZERO;
static addr_t dst_addr_ldc_y                = VAL_ZERO;
static addr_t dst_addr_ldc_uv               = VAL_ZERO;
static addr_t dl_addr_ldc_y                 = VAL_ZERO;
static addr_t dl_addr_ldc_uv                = VAL_ZERO;

static st_imrdrv_attr_param_t      imr_param_ldc_y;
static st_imrdrv_attr_param_t      imr_param_ldc_uv;

static osal_memory_buffer_handle_t dl_buffer_ldc_y;
static osal_memory_buffer_handle_t dl_buffer_ldc_uv;

e_imrdrv_errorcode_t        ret_imr;
osal_memory_buffer_handle_t buf[6];


/**
 * Static Function Declaration
 */
static int R_IMR_AllocImage (st_imrdrv_data_t * img, osal_memory_buffer_handle_t * buf, addr_t * virt_addr, int imr_channel, uint32_t width, uint32_t height, pixfmt_e fmt);
static int R_IMR_AllocDl (st_imrdrv_dl_t * dl, osal_memory_buffer_handle_t * buf, addr_t * virt_addr, int imr_channel, uint32_t alloc_size);
static int R_IMR_TriangleMode (st_imrdrv_triangle_mode_t * triangle_mode);
//add Here


/**
* Function Declaration
*/

int R_IMR_Init (void);
#ifdef NV16 //common.h
int R_IMR_SetupLDC (int format);
#else
int R_IMR_SetupLDC (void);
#endif
int R_IMR_SetupResize (void);
int R_IMR_ExecuteLDC (void);
int R_IMR_ExecuteResize (void);
int R_IMR_Deinit (void);
int R_IMR_AllocImageLDC (void);
int R_IMR_AllocImageResize (void);
void Conv_YUYV2RGB(unsigned char * yuyv, unsigned char * bgr, int width, int height);

int yuv2rgb_main(void *ptr)
{
    Conv_YUYV2RGB(ptr, vin_out_buffer, IMR_Resize_Width, IMR_Resize_Height);                //conversion function
    return 0;
}

void Conv_YUYV2RGB(unsigned char * yuyv, unsigned char * bgr, int width, int height)
{
    int z = 0;
    int x;
    int yline;

    for (yline = 0; yline < height; yline++) {
        for (x = 0; x < width; x++)
        {
            int r, g, b;
            int y, u, v;
            if (!z)
                y = yuyv[0] << 8;
            else
                y = yuyv[2] << 8;

            u = yuyv[1] - 128;
            v = yuyv[3] - 128;

            r = (y + (359 * v)) >> 8;
            g = (y - (88 * u) - (183 * v)) >> 8;
            b = (y + (454 * u)) >> 8;

            *(bgr++) = (unsigned char)((r > 255) ? 255 : ((r < 0) ? 0 : r));
            *(bgr++) = (unsigned char)((g > 255) ? 255 : ((g < 0) ? 0 : g));
            *(bgr++) = (unsigned char)((b > 255) ? 255 : ((b < 0) ? 0 : b));

            if (z++) {
                z = 0;
                yuyv += 4;
            }
        }
    }
}

/**
 * @brief Allocates IMR image
 * @param img  ptr to IMR driver data structure
 * @param buf  ptr to MMGR buffer handle
 * @param imr_channel   Index of IMR channel (req. since physical address uses AXI ID)
 * @param width     Width in pixels
 * @param height    Height in pixels
 * @param fmt       Pixelfmt
 * @return
 */
static int R_IMR_AllocImage(st_imrdrv_data_t * img, osal_memory_buffer_handle_t * buf, addr_t * virt_addr, int imr_channel, uint32_t width, uint32_t height, pixfmt_e fmt)
{
    uintptr_t img_ptr;

    if (!img)
    {
        PRINT_ERROR("Missing img parameter\n");
        return FAILED;
    }

    switch (fmt)
    {
        case PIXFMT_YUYV:
            img->bpp          = IMRDRV_BPP_8;
            img->color_format = IMRDRV_COLOR_FORM_YUYV;
            break;
        case PIXFMT_UV8:
            img->bpp          = IMRDRV_BPP_8;
            img->color_format = IMRDRV_COLOR_FORM_Y;
            break;
        case PIXFMT_SEP_Y:
            img->bpp          = IMRDRV_BPP_8;
            img->color_format = IMRDRV_COLOR_FORM_SEP_Y;
            break;
        case PIXFMT_SEP_UV:
            img->bpp          = IMRDRV_BPP_8;
            img->color_format = /*IMRDRV_COLOR_FORM_SEP_Y;*/IMRDRV_COLOR_FORM_SEP_UV;
            break;
        default:
            PRINT_ERROR("Unsupported pixel format %d\n", fmt);
            return FAILED;
    }

    img->width  = width;
    img->height = height;

    size_t align             = IMR_SRC_ALIGN;
    uint32_t alloc_height    = img->height;
    uint8_t bytes_per_pixel = (img->bpp>IMRDRV_BPP_8) ?((img->bpp==IMRDRV_BPP_32)?4:2):1;

    /* Stride in pixels */

    if (img->width  == IMR_STRIDE_RS)
    {
        img->stride = IMR_STRIDE_RS;
    }
    else
    {
        img->stride = (((((img->width * bytes_per_pixel) - 1) / (uint32_t)align) + 1) * (uint32_t)align);
    }
    printf("img->stride: %d\n", img->stride);
    size_t alloc_size        = (size_t)img->stride * (size_t)alloc_height;
    alloc_size              *= (pixfmt == PIXFMT_YUYV) ? BPP_YUV422 : (pixfmt == PIXFMT_UV8) ? BPP_YUV420 : BPP_Y;// BPP_YUV420;
    DEBUG_PRINT("Alloc_size = [%ld]\n", alloc_size);

    /* allocate buffer via MMGR */
    int res = mmgr_helper_alloc_buffer(buf, imr_channel, (uint32_t)alloc_size, (uint32_t)align, virt_addr, &img_ptr);

    if (res)
    {
        PRINT_ERROR("Failed to allocate %d bytes for image with size %d x %d fmt %d align %d\n", (uint32_t)alloc_size, img->width, img->height, (uint32_t)fmt, (uint32_t)align);
        return FAILED;
    }

    img->phys_addr  = (uint32_t)img_ptr;
    DEBUG_PRINT("Color Format = %d\n", img->color_format);
    return SUCCESS;
}

/**
 * @brief Allocates DL storage
 * @param dl
 * @param virt_addr
 * @param buf
 * @param imr_channel
 * @param size
 * @return
 */
static int R_IMR_AllocDl(st_imrdrv_dl_t * dl, osal_memory_buffer_handle_t * buf, addr_t* virt_addr, int imr_channel, uint32_t alloc_size)
{
    uintptr_t dl_ptr;

    if (!dl)
    {
        PRINT_ERROR("Missing img parameter\n");
        return FAILED;
    }

    /* allocate buffer via MMGR */
    int res = mmgr_helper_alloc_buffer(buf, imr_channel, alloc_size, IMR_DL_ALIGN, virt_addr, &dl_ptr);

    if ((res) || (dl_ptr > 0xFFFFFFFF) || ( *virt_addr == 0))
    {
        PRINT_ERROR("Failed to allocate %d bytes for dl\n", alloc_size);
        return FAILED;
    }

    dl->phys_addr = (uint32_t)dl_ptr;
    return SUCCESS;
}

/**
 * @brief Allocates DL storage
 * @param dl
 * @param virt_addr
 * @param buf
 * @param imr_channel
 * @param size
 * @return
 */

static int R_IMR_TriangleMode(st_imrdrv_triangle_mode_t * triangle_mode)
{

    if (!triangle_mode)
    {
        PRINT_ERROR("Missing img parameter\n");
        return FAILED;
    }

    /* init triangle mode */
    triangle_mode->uvshfval = IMRDRV_MODE_OFF;
    triangle_mode->shfval   = IMRDRV_MODE_OFF;
    triangle_mode->uvshfe   = IMRDRV_MODE_OFF;
    triangle_mode->shfe     = IMRDRV_MODE_OFF;
    triangle_mode->rde      = IMRDRV_MODE_OFF;
    triangle_mode->tfe      = IMRDRV_MODE_OFF;
    triangle_mode->tcm      = IMRDRV_MODE_OFF;
    triangle_mode->autosg   = IMRDRV_MODE_OFF;
    triangle_mode->autodg   = IMRDRV_MODE_OFF;
    triangle_mode->bfe      = IMRDRV_MODE_ON;
    triangle_mode->tme      = IMRDRV_MODE_ON;

    return SUCCESS;
}

int R_IMR_Init(void)
{
    int ret = INVALID;
    #ifdef NV16
    {
        inputWidth_Y       = g_frame_width;
        inputHeight_Y      = g_frame_height;
        inputWidth_UV      = g_frame_width;
        inputHeight_UV     = g_frame_height;

        outputWidth_Y      = IMR_Resize_Width;
        outputHeight_Y     = IMR_Resize_Height;
        outputWidth_UV     = IMR_Resize_Width;
        outputHeight_UV    = IMR_Resize_Height;
    }
    #else
    {
        inputWidth      = g_frame_width;
        inputHeight     = g_frame_height;
        outputWidth     = g_frame_width;
        outputHeight    = g_frame_height;
    }
    #endif
    e_osal_return_t ret_osal;

    ret = init_osal();

    if (ret)
    {
        PRINT_ERROR("Failed init_osal \n");
        return FAILED;
    }

    ret = init_imrdrv();

    if (ret)
    {
        PRINT_ERROR("Failed init_imrdrv \n");
        return FAILED;
    }

    {
        ret = R_IMR_AllocImageLDC();

        if (FAILED == ret)
        {
            PRINT_ERROR("Failed R_IMR_AllocImageLDC \n");
            return FAILED;
        }
    }
    #ifndef NV16
    ret = R_IMR_AllocImageResize();

    if (FAILED == ret)
    {
        PRINT_ERROR("Failed R_IMR_AllocImageResize \n");
        return FAILED;
    }
    #endif

  #ifdef IMR_SAVE_FILE
    char r[50] = {};
	sprintf(r, "%s", "/home/root/IMRLDC.yuv");
	remove(r);
	if (NULL == (IMR_FD = fopen(r, "wb"))) {
        printf("open imr ldc save file failed\n");
    }
  #endif    // IMR_SAVE_FILE


    return SUCCESS;
}

#ifdef NV16
int R_IMR_AllocImageLDC(void)
{
    int ret = INVALID;

    ret = R_IMR_AllocImage(&imr_param_ldc_y.src_data, &buf[0], &src_addr_ldc_y, IMR_Channel, (uint32_t)inputWidth_Y, (uint32_t)inputHeight_Y, (uint32_t)PIXFMT_SEP_Y);
    if (ret)
    {
        PRINT_ERROR("Failed R_IMR_AllocImage LDC ret=%d\n", ret);
        return FAILED;
    }
    ret = R_IMR_AllocImage(&imr_param_ldc_y.dst_data, &buf[1], &dst_addr_ldc_y, IMR_Channel, (uint32_t)outputWidth_Y, (uint32_t)outputHeight_Y, (uint32_t)PIXFMT_SEP_Y);
    if (ret)
    {
        PRINT_ERROR("Failed R_IMR_AllocImage LDC ret=%d\n", ret);
        return FAILED;
    }

    ret = R_IMR_AllocImage(&imr_param_ldc_uv.src_data, &buf[2], &src_addr_ldc_uv, IMR_Channel, (uint32_t)inputWidth_UV, (uint32_t)inputHeight_UV, (uint32_t)PIXFMT_SEP_UV);
    if (ret)
    {
        PRINT_ERROR("Failed R_IMR_AllocImage LDC ret=%d\n", ret);
        return FAILED;
    }
    ret = R_IMR_AllocImage(&imr_param_ldc_uv.dst_data, &buf[3], &dst_addr_ldc_uv, IMR_Channel, (uint32_t)outputWidth_UV, (uint32_t)outputHeight_UV, (uint32_t)PIXFMT_SEP_UV);
    if (ret)
    {
        PRINT_ERROR("Failed R_IMR_AllocImage LDC ret=%d\n", ret);
        return FAILED;
    }

    return SUCCESS;
}
#else
int R_IMR_AllocImageLDC(void)
{
    int ret = INVALID;

    ret     = R_IMR_AllocImage(&imr_param_ldc.src_data, &buf[0], &src_addr_ldc, IMR_Channel, (uint32_t)inputWidth, (uint32_t)inputHeight, (uint32_t)pixfmt);

    if (ret)
    {
        PRINT_ERROR("Failed R_IMR_AllocImage LDC ret=%d\n", ret);
        return FAILED;
    }

    ret = R_IMR_AllocImage(&imr_param_ldc.dst_data, &buf[1], &dst_addr_ldc, IMR_Channel, (uint32_t)outputWidth, (uint32_t)outputHeight, (uint32_t)pixfmt);

    if (ret)
    {
        PRINT_ERROR("Failed R_IMR_AllocImage LDC ret=%d\n", ret);
        return FAILED;
    }

    return SUCCESS;
}
#endif

int R_IMR_AllocImageResize(void)
{

    int ret = INVALID;

    ret = R_IMR_AllocImage(&imr_param_scaling.src_data, &buf[2], &src_addr_scaling, IMR_Channel, (uint32_t)inputWidth, (uint32_t)inputHeight, (uint32_t)pixfmt);

    if (ret)
    {
        PRINT_ERROR("Failed R_IMR_AllocImage Resize ret=%d\n", ret);
        return FAILED;
    }

    ret = R_IMR_AllocImage(&imr_param_scaling.dst_data, &buf[3], &dst_addr_scaling, IMR_Channel, 224, 224, (uint32_t)pixfmt);

    if (ret)
    {
        PRINT_ERROR("Failed R_IMR_AllocImage Resize ret=%d\n", ret);
        return FAILED;
    }

    return SUCCESS;
}

#define IMR_LDC_Params_k1   0.4060391299f
#define IMR_LDC_Params_k2   0
#define IMR_LDC_Params_k3   0
#define IMR_LDC_Params_fx   0.26f
#define IMR_LDC_Params_cx   0.5f
#define IMR_LDC_Params_fy   0.42f
#define IMR_LDC_Params_cy   0.5f

#ifdef NV16 //common.h
int R_IMR_SetupLDC(int format)
{
    int ret = INVALID;
    st_imrdrv_attr_param_t* imr_param_ldc;
    
    if (format == PIXFMT_SEP_Y) {
        DEBUG_PRINT("Set DL for LDC Y plane\n");
        imr_param_ldc = &imr_param_ldc_y;
    }
    else if (format == PIXFMT_SEP_UV) {
        DEBUG_PRINT("Set DL for LDC UV plane\n");
        imr_param_ldc = &imr_param_ldc_uv;
    }

    printf("imr_param_ldc ==>\n");
    printf("src_data.width:  %lu\n", imr_param_ldc->src_data.width);
    printf("src_data.height: %lu\n", imr_param_ldc->src_data.height);
    printf("dst_data.width:  %lu\n", imr_param_ldc->dst_data.width);
    printf("dst_data.height: %lu\n", imr_param_ldc->dst_data.height);

    // /* Set up map function data */
    // opencv_coeff.k1 = IMR_LDC_Params_k1;
    // opencv_coeff.k2 = IMR_LDC_Params_k2;
    // opencv_coeff.k3 = IMR_LDC_Params_k3;
    // opencv_coeff.fx = IMR_LDC_Params_fx * (float)imr_param_ldc->src_data.width;
    // opencv_coeff.cx = IMR_LDC_Params_cx * (float)imr_param_ldc->src_data.width;
    // opencv_coeff.fy = IMR_LDC_Params_fy * (float)imr_param_ldc->src_data.height;
    // opencv_coeff.cy = IMR_LDC_Params_cy * (float)imr_param_ldc->src_data.height;
    // opencv_coeff.sx = ((float)imr_param_ldc->src_data.width) / ((float)imr_param_ldc->dst_data.width);
    // opencv_coeff.sy = ((float)imr_param_ldc->src_data.height) / ((float)imr_param_ldc->dst_data.height);
    // map_func_data   = (void *)&opencv_coeff;

    // /* Set up mapping function */
    // map_func = CustomMapLDC;
    map_coeff.w1 = (float)g_frame_width / (float)IMR_Resize_Width;
    map_coeff.w2 = (float)g_frame_height / (float)IMR_Resize_Height;
    // map_coeff.w1 = 1;
    // map_coeff.w2 = 1;
    /* Set up mapping function */
    map_func_data                         = (void *)&map_coeff;
    /* Set up mapping function */
    map_func                              = CustomMapResize;

    uint32_t stride_size               = BPP_Y;
    imr_param_ldc->src_data.stride     = (uint32_t)imr_param_ldc->src_data.stride * stride_size;
    imr_param_ldc->dst_data.stride     = (uint32_t)imr_param_ldc->dst_data.stride * stride_size;
    DEBUG_PRINT("%s plane stride IMR LDC = %d\n", (format == PIXFMT_SEP_Y) ? "Y" : "UV", imr_param_ldc->src_data.stride);

    st_imrdlg_imr_properties_t iprops =
    {
        .target_device           = IMRDLG_TARGET_V3H_2_0,
        .frame_format            = /*IMRDLG_FMT_Y8,*/(format == PIXFMT_SEP_Y) ? IMRDLG_FMT_Y8 : IMRDLG_FMT_UV8_422,
        .input_frame_width       = (uint16_t)imr_param_ldc->src_data.width,
        .input_frame_height      = (uint16_t)imr_param_ldc->src_data.height,
        .input_frame_stride      = (uint32_t)imr_param_ldc->src_data.stride,
        .output_frame_width      = (uint16_t)imr_param_ldc->dst_data.width,
        .output_frame_height     = (uint16_t)imr_param_ldc->dst_data.height,
        .output_frame_stride     = (uint32_t)imr_param_ldc->dst_data.stride,
        .output_frame_rot_stride = VAL_ZERO,
        .mesh_size               = (uint8_t)mesh_size,
        .mesh_size_limit         = (uint8_t)mesh_size_limit,
        .tra_rot_enable          = IMRDLG_RS_DISABLED,
        .tra_rot_flags           = VAL_ZERO,
        .dl_gen_flags            = IMRDLG_F_ADD_SYNCTRAP,
        .wup_slp_bits            = VAL_ONE,
    };

    printf("input_frame_width:    %u\n", iprops.input_frame_width);
    printf("input_frame_height:   %u\n", iprops.input_frame_height);
    printf("input_frame_stride:   %lu\n", iprops.input_frame_stride);
    printf("output_frame_width:   %u\n", iprops.output_frame_width);
    printf("output_frame_height:  %u\n", iprops.output_frame_height);
    printf("output_frame_stride:  %lu\n", iprops.output_frame_stride);

    e_imrdlg_result_t        dlres;
    st_imrdlg_display_list_t dlout;

    /* Setup Display List */
    dlout.dis_list_size  = 1024UL * 1024UL * 64UL / 4UL;                                                              // 64 MB (MAX allowed by library!)
    dlout.dis_list_pos   = VAL_ZERO;
    int res;
    if (format == PIXFMT_SEP_Y)
        res = allocate_dl_memory(&dl_buffer_ldc_y, dlout.dis_list_size, IMR_DL_ALIGN, (void **)(&dlout.p_dis_list_address));
    else if (format == PIXFMT_SEP_UV)
        res = allocate_dl_memory(&dl_buffer_ldc_uv, dlout.dis_list_size, IMR_DL_ALIGN, (void **)(&dlout.p_dis_list_address));
    if (res)
    {
        PRINT_ERROR("Failed Allocate DL Temp Memory ret=%d \n", res);
        return FAILED;
    }
    else
    {
        DEBUG_PRINT("[SUCCESS] Allocate DL Temp Memory\n");
    }

    DEBUG_PRINT(" Creating DL ...\n");
    dlres = R_IMRDLG_GenerateDisplayList(IMRDLG_ALGO_TMG01, &iprops, NULL, map_func, map_func_data, &dlout);
    if (dlres != 0)
    {
        PRINT_ERROR("DL Creation failed with error %d\n", dlres);
        return FAILED;
    }

    DEBUG_PRINT("dloutsize %d\n", dlout.dis_list_size);

    /* Input image flush to dram */
    if (format == PIXFMT_SEP_Y) {
        mmgr_helper_flush(buf[0]);
        mmgr_helper_flush(buf[1]);
    }
    else if (format == PIXFMT_SEP_UV) {
        mmgr_helper_flush(buf[2]);
        mmgr_helper_flush(buf[3]);
    }

    /* memset and (flush and) invalidate */
    if (format == PIXFMT_SEP_Y)
        memset((void *)dst_addr_ldc_y, 0x00, imr_param_ldc->dst_data.stride * imr_param_ldc->dst_data.height);
    else if (format == PIXFMT_SEP_UV)
        memset((void *)dst_addr_ldc_uv, 0x00, imr_param_ldc->dst_data.stride * imr_param_ldc->dst_data.height);

    if (format == PIXFMT_SEP_Y)
        ret = R_IMR_AllocDl(&(imr_param_ldc->dl_data), &buf[5], &dl_addr_ldc_y, IMR_Channel, 16 * 1024 * 1024);
    else if (format == PIXFMT_SEP_UV)
        ret = R_IMR_AllocDl(&(imr_param_ldc->dl_data), &buf[4], &dl_addr_ldc_uv, IMR_Channel, 16 * 1024 * 1024);
    if (ret)
    {
        PRINT_ERROR("Failed R_IMR_AllocDl %d\n", ret);
        return FAILED;
    }

    ret = R_IMR_TriangleMode(&(imr_param_ldc->triangle_mode));
    if (ret)
    {
        PRINT_ERROR("Failed R_IMR_TriangleMode %d\n", ret);
        return FAILED;
    }

    /* Copy over DL and flush to memory */
    if (format == PIXFMT_SEP_Y) {
        memcpy((void *)dl_addr_ldc_y, dlout.p_dis_list_address, dlout.dis_list_pos);
        mmgr_helper_flush(buf[5]);
    }
    else if (format == PIXFMT_SEP_UV) {
        memcpy((void *)dl_addr_ldc_uv, dlout.p_dis_list_address, dlout.dis_list_pos);
        mmgr_helper_flush(buf[4]);
    }

    return SUCCESS;
}
#else
int R_IMR_SetupLDC(void)
{
    int ret = INVALID;
    /* Set up map function data */
    opencv_coeff.k1 = IMR_LDC_Params_k1;
    opencv_coeff.k2 = IMR_LDC_Params_k2;
    opencv_coeff.k3 = IMR_LDC_Params_k3;
    opencv_coeff.fx = IMR_LDC_Params_fx * (float)inputWidth;
    opencv_coeff.cx = IMR_LDC_Params_cx * (float)inputWidth;
    opencv_coeff.fy = IMR_LDC_Params_fy * (float)inputHeight;
    opencv_coeff.cy = IMR_LDC_Params_cy * (float)inputHeight;
    opencv_coeff.sx = ((float)inputWidth) / ((float)outputWidth);
    opencv_coeff.sy = ((float)inputHeight) / ((float)outputHeight);
    map_func_data   = (void *)&opencv_coeff;

    /* Set up mapping function */
    map_func = CustomMapLDC;

    // // map_coeff.w1 = (float)g_frame_width / (float)IMR_Resize_Width;
    // // map_coeff.w2 = (float)g_frame_height / (float)IMR_Resize_Height;
    // map_coeff.w1 = 1;
    // map_coeff.w2 = 1;
    // /* Set up mapping function */
    // map_func_data                         = (void *)&map_coeff;
    // /* Set up mapping function */
    // map_func                              = CustomMapResize;

    uint32_t stride_size              = (pixfmt == PIXFMT_YUYV) ? BPP_YUV422 : (pixfmt == PIXFMT_UV8) ? BPP_YUV420 : BPP_Y;
    //uint32_t stride_size              = BPP_YUV420;
    imr_param_ldc.src_data.stride     = (uint32_t)imr_param_ldc.src_data.stride * stride_size;
    imr_param_ldc.dst_data.stride     = (uint32_t)imr_param_ldc.dst_data.stride * stride_size;
    DEBUG_PRINT("Image stride IMR LDC = %d\n", imr_param_ldc.src_data.stride);
    //printf("%d, %d, %d\n", imr_param_ldc.dst_data.width, imr_param_ldc.dst_data.height, imr_param_ldc.dst_data.stride);
    st_imrdlg_imr_properties_t iprops =
    {
        .target_device           = IMRDLG_TARGET_V3H_2_0,
        .frame_format            = IMRDLG_FMT_YUYV, // IMRDLG_FMT_UV8_420, IMRDLG_FMT_YUYV,
        .input_frame_width       = (uint16_t)imr_param_ldc.src_data.width,
        .input_frame_height      = (uint16_t)imr_param_ldc.src_data.height,
        .input_frame_stride      = (uint32_t)imr_param_ldc.src_data.stride,
        .output_frame_width      = (uint16_t)imr_param_ldc.dst_data.width,
        .output_frame_height     = (uint16_t)imr_param_ldc.dst_data.height,
        .output_frame_stride     = (uint32_t)imr_param_ldc.dst_data.stride,
        .output_frame_rot_stride = VAL_ZERO,
        .mesh_size               = (uint8_t)mesh_size,
        .mesh_size_limit         = (uint8_t)mesh_size_limit,
        .tra_rot_enable          = IMRDLG_RS_DISABLED,
        .tra_rot_flags           = VAL_ZERO,
        .dl_gen_flags            = IMRDLG_F_ADD_SYNCTRAP,
        .wup_slp_bits            = VAL_ONE,
    };

    e_imrdlg_result_t        dlres;
    st_imrdlg_display_list_t dlout;

    /* Setup Display List */
    dlout.dis_list_size  = 1024UL * 1024UL * 64UL / 4UL;                                                              // 64 MB (MAX allowed by library!)
    dlout.dis_list_pos   = VAL_ZERO;
    int res              = allocate_dl_memory(&dl_buffer_ldc, dlout.dis_list_size, IMR_DL_ALIGN, (void **)(&dlout.p_dis_list_address));

    if (res)
    {
        PRINT_ERROR("Failed Allocate DL Temp Memory ret=%d \n", res);
        return FAILED;
    }
    else
    {
        DEBUG_PRINT("[SUCCESS] Allocate DL Temp Memory\n");
    }

    DEBUG_PRINT(" Creating DL ...\n");
    dlres = R_IMRDLG_GenerateDisplayList(IMRDLG_ALGO_TMG01, &iprops, NULL, map_func, map_func_data, &dlout);

    if (dlres != 0)
    {
        PRINT_ERROR("DL Creation failed with error %d\n", dlres);
        return FAILED;
    }

    DEBUG_PRINT("dloutsize %d\n", dlout.dis_list_size);

    /* Input image flush to dram */
    mmgr_helper_flush(buf[0]);
    /* memset and (flush and) invalidate */
    memset((void *)dst_addr_ldc, 0x00, imr_param_ldc.dst_data.stride * imr_param_ldc.dst_data.height);

    ret = R_IMR_AllocDl(&imr_param_ldc.dl_data, &buf[4], &dl_addr_ldc, IMR_Channel, 16 * 1024 * 1024);

    if (ret)
    {
        PRINT_ERROR("Failed R_IMR_AllocDl %d\n", ret);
        return FAILED;
    }

    ret = R_IMR_TriangleMode(&imr_param_ldc.triangle_mode);

    if (ret)
    {
        PRINT_ERROR("Failed R_IMR_TriangleMode %d\n", ret);
        return FAILED;
    }

    /* Copy over DL and flush to memory */
    memcpy((void *)dl_addr_ldc, dlout.p_dis_list_address, dlout.dis_list_pos);
    mmgr_helper_flush(buf[4]);

    return SUCCESS;

}
#endif

int R_IMR_SetupResize(void)
{
    int ret = INVALID;

    map_coeff.w1 = (float)g_frame_width / (float)IMR_Resize_Width;
    map_coeff.w2 = (float)g_frame_height / (float)IMR_Resize_Height;
    /* Set up mapping function */
    map_func_data                         = (void *)&map_coeff;
    /* Set up mapping function */
    map_func                              = CustomMapResize;

    uint32_t stride_size                  = (pixfmt == PIXFMT_YUYV) ? BPP_YUV422 : (pixfmt == PIXFMT_UV8) ? BPP_YUV420 : BPP_Y;
    //uint32_t stride_size                  = BPP_YUV420;
    imr_param_scaling.src_data.stride     = (uint32_t)imr_param_scaling.src_data.stride * stride_size;
    imr_param_scaling.dst_data.stride     = (uint32_t)imr_param_scaling.dst_data.stride * stride_size;
    DEBUG_PRINT("Image stride IMR Scaling = %d\n", imr_param_scaling.src_data.stride);

    st_imrdlg_imr_properties_t iprops =
    {
        .target_device           = IMRDLG_TARGET_V3H_2_0,
        .frame_format            = IMRDLG_FMT_YUYV, //IMRDLG_FMT_UV8_420, MRDLG_FMT_YUYV,
        .input_frame_width       = (uint16_t)imr_param_scaling.src_data.width,
        .input_frame_height      = (uint16_t)imr_param_scaling.src_data.height,
        .input_frame_stride      = (uint32_t)imr_param_scaling.src_data.stride,
        .output_frame_width      = (uint16_t)imr_param_scaling.dst_data.width,
        .output_frame_height     = (uint16_t)imr_param_scaling.dst_data.height,
        .output_frame_stride     = (uint32_t)imr_param_scaling.dst_data.stride,
        .output_frame_rot_stride = 0,
        .mesh_size               = (uint8_t)mesh_size,
        .mesh_size_limit         = (uint8_t)mesh_size_limit,
        .tra_rot_enable          = IMRDLG_RS_DISABLED,
        .tra_rot_flags           = 0,
        .dl_gen_flags            = IMRDLG_F_ADD_SYNCTRAP,
        .wup_slp_bits            = 0,
    };

    e_imrdlg_result_t        dlres;
    st_imrdlg_display_list_t dlout;
    /* Setup DL */
    dlout.dis_list_size = 1024UL * 1024UL * 64UL / 4UL;                                                              // 64 MB (MAX allowed by library!)
    dlout.dis_list_pos  = 0;

    ret = allocate_dl_memory(&dl_buffer_scaling, dlout.dis_list_size, IMR_DL_ALIGN, (void **)(&dlout.p_dis_list_address));

    if (ret)
    {
        PRINT_ERROR("Failed Allocate DL Temp Memory ret=%d \n", ret);
        return FAILED;
    }
    else
    {
        DEBUG_PRINT("[SUCCESS] Allocate DL Temp Memory\n");
    }

    DEBUG_PRINT(" Creating Display List for Resize \n");


    dlres = R_IMRDLG_GenerateDisplayList(IMRDLG_ALGO_TMG01, &iprops, NULL, map_func, map_func_data, &dlout);
    DEBUG_PRINT("dlout size %d\n", dlout.dis_list_size);

    if (dlres != 0)
    {
        PRINT_ERROR("Failed R_IMRDLG_GenerateDisplayList for resize %d\n", dlres);
        return FAILED;
    }

    mmgr_helper_flush(buf[2]);
    memset((void *)dst_addr_scaling, 0x00, imr_param_scaling.dst_data.stride * imr_param_scaling.dst_data.height);

    ret = R_IMR_AllocDl(&imr_param_scaling.dl_data, &buf[5], &dl_addr_scaling, IMR_Channel, 16 * 1024 * 1024);

    if (ret)
    {
        PRINT_ERROR("Failed R_IMR_AllocDl for resize %d\n", ret);
        return FAILED;
    }

    ret = R_IMR_TriangleMode(&imr_param_scaling.triangle_mode);

    if (ret)
    {
        PRINT_ERROR("Failed R_IMR_TriangleMode for resize %d\n", ret);
        return FAILED;
    }

    memcpy((void *)dl_addr_scaling, dlout.p_dis_list_address, dlout.dis_list_pos);
    mmgr_helper_flush(buf[5]);

    return SUCCESS;
}

#ifdef NV16 //common.h
unsigned char tmp[g_frame_width * g_frame_height * 2] = {};
int R_IMR_ExecuteLDC(void)
{
    int ret = INVALID;

    DEBUG_PRINT("running IMR-LDC \n");
    memset(tmp, 0, g_frame_width * g_frame_height * 2);
    memcpy(tmp, imr_ldc_in, g_frame_width * g_frame_height * 2);
    memcpy((void *)src_addr_ldc_y, (void *)tmp, g_frame_width * g_frame_height);
    memcpy((void *)src_addr_ldc_uv, (void *)(tmp + g_frame_width * g_frame_height), g_frame_width * g_frame_height);

    //memcpy((void *)src_addr_ldc, (void *)imr_ldc_in, g_frame_width * g_frame_height * BPP_YUV422);
    // memcpy((void *)src_addr_ldc, (void *)imr_ldc_in, g_frame_width * g_frame_height * BPP_YUV420);

    imrdrv_ctrl_handle_t imr_handles_ch0_y;
    imrdrv_ctrl_handle_t imr_handles_ch0_uv;

    for (int i=0 ; i<2 ; i++)
{
    if (i==0) {
        imr_handles_ch0_y = get_imrdrv_ctrlhandles(IMR_Channel);
        ret_imr = R_IMRDRV_AttrSetParam(imr_handles_ch0_y, &imr_param_ldc_y);
    }
    else if (i==1) {
        imr_handles_ch0_uv = get_imrdrv_ctrlhandles(IMR_Channel);
        if (imr_handles_ch0_uv == NULL)
            PRINT_ERROR("imr_handles_ch0_uv == NULL !!!\n");
        ret_imr = R_IMRDRV_AttrSetParam(imr_handles_ch0_uv, &imr_param_ldc_uv);
    }
    if (ret_imr != IMRDRV_ERROR_OK)
    {
        PRINT_ERROR("Failed R_IMRDRV_AttrSetParam for ldc %s ch=%d ret=%d\n", (i==0) ? "Y plane" : (i==1) ? "UV plane" : "unknow", IMR_Channel, ret_imr);
        return FAILED;
    }

    DEBUG_PRINT("Succeed R_IMRDRV_AttrSetParam ch=%d\n", IMR_Channel);

        /* ---- Setting Sync ------*/
    st_imrdrv_attr_cache_mode_t cache_mode_param;

    if (enable_prefetch == 1)
    {
          cache_mode_param.cache_mode = IMRDRV_CACHE_MODE_NON_BLOCKING;
    }
    else
    {
          cache_mode_param.cache_mode = IMRDRV_CACHE_MODE_NORMAL;
    }

    cache_mode_param.double_cache_mode = IMRDRV_DOUBLE_CACHE_MODE_SINGLE;

    ret_imr = R_IMRDRV_AttrSetCacheMode((i==0) ? imr_handles_ch0_y : imr_handles_ch0_uv, &cache_mode_param);

    if (ret_imr != IMRDRV_ERROR_OK)
    {
        PRINT_ERROR("Failed R_IMRDRV_AttrSetCacheMode LDC ret=%d\n", ret_imr);

        if (ret != OSAL_RETURN_OK)
        {
            PRINT_ERROR("Failed R_IMRDRV_AttrSetCacheMode LDC ret=%d\n", ret);
        }
        return FAILED;
    }

    ret_imr =  R_IMRDRV_Execute((i==0) ? imr_handles_ch0_y : imr_handles_ch0_uv);

    if (ret_imr != IMRDRV_ERROR_OK)
    {
        PRINT_ERROR("Failed R_IMRDRV_Execute LDC ret=%d\n", ret_imr);
        return FAILED;
    }

    DEBUG_PRINT("Succeed R_IMRDRV_Execute LDC ch = %d\n", IMR_Channel);

    ret = imrdrv_wait();

    if (ret)
    {
        PRINT_ERROR("***ERROR: imr_exectute LDC failed with %d\n", ret);
        return FAILED;
    }
    //R_OSAL_ThreadSleepForTimePeriod ((osal_milli_sec_t)2);
}

    
    FILE* file = fopen("test_y.yuv","wb");
    fwrite((void*)dst_addr_ldc_y, sizeof(uint8_t), outputWidth_Y * outputHeight_Y, file);
    fclose(file);

    file = fopen("test_uv.yuv","wb");
    fwrite((void*)dst_addr_ldc_uv, sizeof(uint8_t), outputWidth_UV * outputHeight_UV, file);
    fclose(file);
    // memcpy((void *)src_addr_scaling, (void *)dst_addr_ldc, g_frame_width * g_frame_height * BPP_YUV420);
    // memcpy((void *)imr_ldc_buffer, (void *)dst_addr_ldc, g_frame_width * g_frame_height * BPP_YUV420);
    //memcpy((void *)src_addr_scaling, (void *)dst_addr_ldc, g_frame_width * g_frame_height * BPP_YUV422);
    //memcpy((void *)imr_ldc_buffer, (void *)dst_addr_ldc, g_frame_width * g_frame_height * BPP_YUV422);

    // yuv2rgb_main((void *)dst_addr_ldc);

  #ifdef IMR_SAVE_FILE
    ssize_t  bytes;
	bytes = fwrite (dst_addr_ldc, sizeof(uint8_t), g_frame_width * g_frame_height * BPP_YUV420, IMR_FD);
	fflush(IMR_FD);
  #endif    // IMR_SAVE_FILE

    return SUCCESS;
}

#else
int R_IMR_ExecuteLDC(void)
{
    int ret = INVALID;

    DEBUG_PRINT("running IMR-LDC \n");


    memcpy((void *)src_addr_ldc, (void *)imr_ldc_in, g_frame_width * g_frame_height * BPP_YUV422);
    // memcpy((void *)src_addr_ldc, (void *)imr_ldc_in, g_frame_width * g_frame_height * BPP_YUV420);

    imrdrv_ctrl_handle_t imr_handles_ch0 = get_imrdrv_ctrlhandles(IMR_Channel);

    ret_imr = R_IMRDRV_AttrSetParam(imr_handles_ch0, &imr_param_ldc);

    if (ret_imr != IMRDRV_ERROR_OK)
    {
        PRINT_ERROR("Failed R_IMRDRV_AttrSetParam for resize ch=%d ret=%d\n", IMR_Channel, ret_imr);
        return FAILED;
    }

    DEBUG_PRINT("Succeed R_IMRDRV_AttrSetParam ch=%d\n", IMR_Channel);

        /* ---- Setting Sync ------*/
    st_imrdrv_attr_cache_mode_t cache_mode_param;

    if (enable_prefetch == 1)
    {
          cache_mode_param.cache_mode = IMRDRV_CACHE_MODE_NON_BLOCKING;
    }
    else
    {
          cache_mode_param.cache_mode = IMRDRV_CACHE_MODE_NORMAL;
    }

    cache_mode_param.double_cache_mode = IMRDRV_DOUBLE_CACHE_MODE_SINGLE;

    ret_imr = R_IMRDRV_AttrSetCacheMode(imr_handles_ch0, &cache_mode_param);

    if (ret_imr != IMRDRV_ERROR_OK)
    {
        PRINT_ERROR("Failed R_IMRDRV_AttrSetCacheMode LDC ret=%d\n", ret_imr);

        if (ret != OSAL_RETURN_OK)
        {
            PRINT_ERROR("Failed R_IMRDRV_AttrSetCacheMode LDC ret=%d\n", ret);
        }
        return FAILED;
    }

    ret_imr =  R_IMRDRV_Execute(imr_handles_ch0);

    if (ret_imr != IMRDRV_ERROR_OK)
    {
        PRINT_ERROR("Failed R_IMRDRV_Execute LDC ret=%d\n", ret_imr);
        return FAILED;
    }

    DEBUG_PRINT("Succeed R_IMRDRV_Execute LDC ch = %d\n", IMR_Channel);

    ret = imrdrv_wait();

    if (ret)
    {
        PRINT_ERROR("***ERROR: imr_exectute LDC failed with %d\n", ret);
        return FAILED;
    }
    
    // FILE* file = fopen("test.yuv","wb");
    // fwrite((void*)dst_addr_ldc, sizeof(uint8_t), outputWidth * outputHeight * BPP_YUV422, file);
    // fclose(file);

    // memcpy((void *)src_addr_scaling, (void *)dst_addr_ldc, g_frame_width * g_frame_height * BPP_YUV420);
    // memcpy((void *)imr_ldc_buffer, (void *)dst_addr_ldc, g_frame_width * g_frame_height * BPP_YUV420);
    //memcpy((void *)src_addr_scaling, (void *)dst_addr_ldc, g_frame_width * g_frame_height * BPP_YUV422);
    memcpy((void *)imr_ldc_buffer, (void *)dst_addr_ldc, g_frame_width * g_frame_height * BPP_YUV422);

    // yuv2rgb_main((void *)dst_addr_ldc);

  #ifdef IMR_SAVE_FILE
    ssize_t  bytes;
	bytes = fwrite (dst_addr_ldc, sizeof(uint8_t), g_frame_width * g_frame_height * BPP_YUV420, IMR_FD);
	fflush(IMR_FD);
  #endif    // IMR_SAVE_FILE

    return SUCCESS;
}
#endif

int R_IMR_ExecuteResize(void)
{
    int ret = INVALID;
    struct timeval  mod_starttime, mod_endtime;

    // memcpy((void *)src_addr_scaling, (void *)imr_res_in, g_frame_width * g_frame_height * BPP_YUV420);
    memcpy((void *)src_addr_scaling, (void *)imr_res_in, g_frame_width * g_frame_height * BPP_YUV422);

    DEBUG_PRINT("running IMR-Resize \n");
    imrdrv_ctrl_handle_t imr_handles_ch4 = get_imrdrv_ctrlhandles(IMR_Channel);

    ret_imr = R_IMRDRV_AttrSetParam(imr_handles_ch4, &imr_param_scaling);

    if (ret_imr != IMRDRV_ERROR_OK)
    {
        PRINT_ERROR("Failed R_IMRDRV_AttrSetParam Resize ch=%d ret=%d\n", IMR_Channel, ret_imr);
        return FAILED;
    }

    DEBUG_PRINT("Succeed R_IMRDRV_AttrSetParam Resize ch=%d\n", IMR_Channel);

    ret_imr =  R_IMRDRV_Execute(imr_handles_ch4);

    if (ret_imr != IMRDRV_ERROR_OK)
    {
        PRINT_ERROR("Failed R_IMRDRV_Execute Resize ret=%d\n", ret_imr);
        return FAILED;
    }

    DEBUG_PRINT("Succeed R_IMRDRV_Execute Resize ch = %d\n", IMR_Channel);

    /* Wait for IMR */
    ret = imrdrv_wait();

    if (ret)
    {
        PRINT_ERROR("ERROR: imr_exectute Resize failed with %d\n", ret);
        return FAILED;
    }

    yuv2rgb_main((void *)dst_addr_scaling);

    DEBUG_PRINT("yuv2rgb_main completed\n");

    return SUCCESS;
}
int R_IMR_Deinit(void)
{
    /* Release DL and Image planes (NOTE: inverse order !) */
    if (SUCCESS == dmsStatus.imr_ldc.status)
    {
        mmgr_helper_dealloc(buf[0]);
        mmgr_helper_dealloc(buf[1]);
		mmgr_helper_dealloc(buf[4]);
        mmgr_helper_dealloc(dl_buffer_ldc);
    }
    if (SUCCESS == dmsStatus.imr_rs.status)
    {
        mmgr_helper_dealloc(buf[2]);
        mmgr_helper_dealloc(buf[3]);
		mmgr_helper_dealloc(buf[5]);
        mmgr_helper_dealloc(dl_buffer_scaling);
    }

    /* shutdown */
    DEBUG_PRINT("Tearing down drivers\n");
    if (SUCCESS == dmsStatus.imr_ldc.status || SUCCESS == dmsStatus.imr_rs.status)
    {
        deinit_imrdrv();
    }
    DEBUG_PRINT("IMR END\n");

  #ifdef IMR_SAVE_FILE
    fclose(IMR_FD);
  #endif    // IMR_SAVE_FILE

    return SUCCESS;
}
