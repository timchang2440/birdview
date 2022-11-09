/*********************************************************************************************************************
* File Name    : vout_main.h
* Version      : 1.0.0
* Product Name : DMS Application
* Device(s)    : R-Car V3h2
* Description  : Video Output header File
*********************************************************************************************************************/

/*********************************************************************************************************************
* History : Version DD.MM.YYYY Description
*         : 1.0.0   20.05.2022 Initial version 
*********************************************************************************************************************/
#ifndef __VOUT_MAIN_H__
#define __VOUT_MAIN_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "common.h"

int vout_init (void);
int execute (void);
int64_t vout_deinit (void);

#ifdef __cplusplus
}
#endif

#endif /* __VOUT_MAIN_H__ */
