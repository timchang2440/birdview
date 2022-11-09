/***********************************************************************************************************************
* File Name    : common.h
* Version      : 1.0.0
* Product Name : DMS Application
* Device(s)    : R-Car V3h2
* Description  : header File
***********************************************************************************************************************/

/***********************************************************************************************************************
* History : Version DD.MM.YYYY Description
*         : 1.0.0   20.05.2022 Initial version
***********************************************************************************************************************/
#ifndef COMMON_H_
#define COMMON_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**********************************************************************************************************************
 Includes   <System Includes> , "Project Includes"
 *********************************************************************************************************************/
#include <sys/time.h>
#include "rcar-xos/osal/r_osal.h"
#include "customize.h"
/**********************************************************************************************************************
 Macro definitions
 *********************************************************************************************************************/
#define NV16 //#Tim 1108 vin_capture.c imr_main.c main.c

#define g_frame_width         ( 1280 )
#define g_frame_height        ( 800 )
#define IMR_Channel           ( 0 )
#define IMR_Resize_Width      ( 1280 )
#define IMR_Resize_Height     ( 800 )

#define OSAL_RESOURCE_ID      ( 0xf000U )
#define MUTEX_ID_NO1          ( OSAL_RESOURCE_ID + 0U )
#define MUTEX_ID_NO2          ( OSAL_RESOURCE_ID + 1U )
#define TIMEOUT_MS            ( 10000 )                         // 10000 milisecond
#define TIMEOUT_1MS_SLEEP     ( 1  )                             // 1 milisecond
#define TIMEOUT_5MS_SLEEP     ( 5  )
#define TIMEOUT_10MS_SLEEP    ( 10 )                            // 10 milisecond
#define TIMEOUT_20MS_SLEEP    ( 20 )                            // 20 milisecond
#define TIMEOUT_25MS_SLEEP    ( 25 )                            // 25 milisecond
#define TIMEOUT_50MS_SLEEP    ( 50 )
#define TIMEOUT_MS_WAIT       ( 5.)
#define DATA_LEN_64           ( 64 )
#define DATA_LEN_128          ( 128 )
#define BPP_YUV422            ( 2 )
#define BPP_YUV420            ( 1.5 )
#define BPP_RGB               ( 3 )
#define BPP_Y                 ( 1 )
#define NUM_CNN_CHANNELS      ( 3 )
#define SUCCESS               ( 0 )
#define FAILED                ( 1 )
#define INVALID               ( -1 )
#define PRINT_ERROR(...)  { printf("ERROR: %s (%d): ", __func__, __LINE__); printf(__VA_ARGS__);}
#define DEBUG_PRINT(...)  { if (false) { printf("DEBUG: %s(%d):", __func__, __LINE__); printf(__VA_ARGS__);} }
#define OSAL_SAMPLE_ERR(...)  {printf("error: %s(%d):", __func__, __LINE__); printf(__VA_ARGS__);}

#define MAX_MODULE            ( 5 )


/**********************************************************************************************************************
 Typedef definitions
 *********************************************************************************************************************/
typedef enum _driverActivity
{
    eNone = -1,
    eDrinkingWater = 0,
    eHairnMakeUp,
    eLookingDown,
    eCallLeft,
    eCallRight,
    eReachBack,
    eReachSide,
    eSafeDrive,
    eTalkintoPassenger
}_driverActivity;

/**********************************************************************************************************************
 Typedef definitions
 *********************************************************************************************************************/
typedef struct _CnnChannelDetails
{
    void *p_virt;
    size_t width;
    size_t height;
    unsigned int bytesPerPixel;
    unsigned int channel;

}_CnnChannelDetails;

/**********************************************************************************************************************
 Typedef definitions
 *********************************************************************************************************************/
typedef struct
{
    int status;
    int is_enable;
}modulestatus;

/**********************************************************************************************************************
 Typedef definitions
 *********************************************************************************************************************/
typedef struct
{
    modulestatus imr_ldc;
    modulestatus imr_rs;
    modulestatus isp;
    modulestatus vin;
    modulestatus vout;
    modulestatus cnn;
}systemstatus;

/**********************************************************************************************************************
 Exported global variables and functions
 *********************************************************************************************************************/
extern systemstatus dmsStatus;
//extern int64_t g_frame_width;
//extern int64_t g_frame_height;
extern uint32_t us_YUVRGB;
extern int det_accuracy ;
extern unsigned char * vin_out_buffer;
extern unsigned char * imr_ldc_buffer;
extern unsigned char * lde_out_buffer;
extern unsigned char * opencv_buffer;
extern unsigned char * cnn_rgb_buffer;
extern unsigned char * isp_in;
extern unsigned char * imr_ldc_in;
extern unsigned char * imr_res_in;
extern unsigned char * lde_in;
extern unsigned char * gst_in;
extern unsigned char * cnn_in;
extern unsigned char * opencv_in;
extern unsigned char * vout_in;
extern void * gp_isp_out_y;
extern void * gp_isp_out_uv;
extern _driverActivity driverActivity;
extern osal_memory_manager_handle_t handle_osalmmngr;
extern st_osal_mmngr_config_t osal_mmngr_config;
extern osal_mutex_handle_t mtx_handle ;
extern _CnnChannelDetails CnnChannelDetails[NUM_CNN_CHANNELS];
extern bool g_debug_save_flag;

extern bool vin_out_buffer_update;

/* VIN */
extern int R_VIN_Initilize();
extern int R_VIN_Execute();
extern int R_VIN_DeInitialize();

/* ISP */
extern int R_ISP_Initialize();
extern int R_ISP_Execute();
extern int R_ISP_DeInitialize();
/* IMR */
extern int R_IMR_Init();
extern int R_IMR_Deinit();
extern int R_IMR_SetupLDC();
extern int R_IMR_SetupResize(void);
extern int R_IMR_ExecuteLDC(void);
extern int R_IMR_ExecuteResize(void);
extern int R_IMR_AllocImageLDC(void);
extern int R_IMR_AllocImageResize(void);
extern int init_mmgr(void);
/* color conv */
extern int y_uv2yuyv(char *pdst, char *py, char *puv, int width, int height);
extern int yuv2rgb_main();
/* VOUT */
extern int vout_init();
extern int execute();
extern int64_t vout_deinit();
/* RIVP LDE */
extern int R_LDE_Init();
extern int R_LDE_Execute();
extern int R_LDE_Deinit();


extern unsigned long GetTimer ();

#ifdef __cplusplus
}
#endif

#endif /* COMMON_H_ */

