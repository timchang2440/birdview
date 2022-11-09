/*********************************************************************************************************************
* File Name    : vin_capture.h
* Version      : 1.0.0
* Product Name : DMS Application
* Device(s)    : R-Car V3h2
* Description  : Video Output header File
*********************************************************************************************************************/

/*********************************************************************************************************************
* History : Version DD.MM.YYYY Description
*         : 1.0.0   20.05.2022 Initial version 
*********************************************************************************************************************/
#ifndef __VIN_CAPTURE_H__
#define __VIN_CAPTURE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "common.h"

int R_VIN_Initialize();
int R_VIN_Open();
int R_VIN_SetUpVideoInput();
int R_VIN_Execute();
int R_VIN_DeInitialize();
int R_VIN_Close();
int64_t R_Capture_Task();

#ifdef __cplusplus
}
#endif

#endif /* __VIN_CAPTURE_H__ */
