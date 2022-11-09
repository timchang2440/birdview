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
/**********************************************************************************************************************
 Macro definitions
 *********************************************************************************************************************/
#ifndef CUSTOMIZE_H_
#define CUSTOMIZE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define DMS_CustomizeFile "config/DMS_Customize.config"

/**********************************************************************************************************************
 Typedef definitions
 *********************************************************************************************************************/
typedef struct {
    /* VIN */
    int VIN_Enable;                             /* 0:Disable, 1:Enable - (default:1)*/
    int VIN_Device;
    int VIN_Capture_Format;                     /* 0:"YUYV", 1:UYVY, 1:RGB24, 2:RAW - (default:0) */
    int VIN_Capture_Width;
    int VIN_Capture_Height;
    int VIN_Offset_X;
    int VIN_Offset_Y;
    int VIN_Req_Buffer_Num;

    /* Image */
    int Frame_Width;
    int Frame_Height;

    /* ISP */
    int ISP_Enable;                             /* 0:Disable, 1:Enable - (default:0) */
    int ISP_Channel;                            /* 0:Channel 0, Channel 1 - (default:0) */
    int ISP_RAW_IN_Format;                      /* 0:"RGGB", 1:BGGR - (default:0) */
    int ISP_RAW_OUT_Format;                     /* 0:"Y+UV", 1:RGB24 - (default:0) */
    /* IMR */
    int IMR_Channel;                            /* 0 ~ 5 (default:0) */
    int IMR_LDC;                                /* 0:Disable, 1:Enable - (default:0) */
    float IMR_LDC_Params_k1;                    /* 0.4060391299 */
    float IMR_LDC_Params_k2;
    float IMR_LDC_Params_k3;
    int IMR_LDC_Params_p1;
    int IMR_LDC_Params_p2;
    float IMR_LDC_Params_fx;                    /* (fx * Input Width) */
    float IMR_LDC_Params_fy;                    /* (fy * Input Height) */
    float IMR_LDC_Params_cx;                    /* (cx * input Width) */
    float IMR_LDC_Params_cy;                    /* (fy * Input Height) */
    int IMR_Resize;                             /* 0:Disable, 1:Enable  - (default:1) */
    int IMR_Resize_Width;                       /* (default:224) */
    int IMR_Resize_Height;                      /* (default:224) */
    /* CNN */
    int CNN_Enable;                             /* 0:Disable, 1:Enable  - (default:1) */
    int CNN_NumberOfChannels;
    char CNN_ChannelName[64];
    char AI_Model_Name[64];
    char Inference_Output_File[64];
    char Inference_File_Format[64];
    /*OpenCV - Post Processing */
    int DMS_Text;                               /* 0:OFF, 1:ON (default:1) */
    int DMS_Text_Position_X;
    int DMS_Text_Position_Y;
    int DMS_Text_Size;
    int DMS_Text_Font;
    char DMS_Text_Color_Safe[64];
    char DMS_Text_Color_Others[64];
    int DMS_Act_SafeDrive;                      /* 0:OFF, 1:ON (default:1) */
    int DMS_Act_Phone_call_Right;               /* 0:OFF, 1:ON (default:1) */
    int DMS_Act_Phone_call_Left;                /* 0:OFF, 1:ON (default:1) */
    int DMS_Act_Drinking;                       /* 0:OFF, 1:ON (default:1) */
    int DMS_Act_Reach_Side;                     /* 0:OFF, 1:ON (default:1) */
    int DMS_Act_Hair_Makeup;                    /* 0:OFF, 1:ON (default:1) */
    int DMS_Act_Talking_Passenger;              /* 0:OFF, 1:ON (default:1) */
    int DMS_Act_Reach_Backseat;                 /* 0:OFF, 1:ON (default:1) */
    int DMS_Act_Look_Down;                      /* 0:OFF, 1:ON (default:1) */
    /* VOUT */
    char DRM_Module[64];
    int VOUT_Display_Format;                    /* 0:"YUYV", 1:UYVY, 1:RGB24 (default:0) */
    int VOUT_Display_Width;
    int VOUT_Display_Height;
    int VOUT_Pos_X;
    int VOUT_Pos_Y;
    /* Debug */
    int Debug_Enable;                            /* 0:OFF, 1:ON (default:1) */
    int Logging;                                 /* 0:OFF, 1:ON (default:1) */
    int Proc_Time;
    /* Frame.file*/
    char Frame_File_Name[64];
}st_customize_t;

/**********************************************************************************************************************
 Exported global variables and functions
 *********************************************************************************************************************/
extern st_customize_t g_customize;
void  R_CustomizeInit(st_customize_t *cf);
int  R_CustomizeLoad(st_customize_t *cf, const char *file_name);
int  R_CustomizePrint(st_customize_t *cf);
int R_CustomizeValidate(st_customize_t *cf);

#ifdef __cplusplus
}
#endif

#endif

