#include "rcar-xos/imr/r_imrdrv_api.h"
#include "rcar-xos/imr/r_imrdlg.h"
#include "rcar-xos/osal/r_osal.h"
#include "rcar-xos/osal/r_osal_memory_impl.h"
#include "rcar-xos/imr/r_imrdrv_type.h"

#include "include/osaldrv_helper.h"
#include "include/imrlxsample_syn.h"
#include "common.h"
#include "customize.h"


/*
 * Forward declaration
 */


static int32_t imrdrv_callback_func (const e_imrdrv_errorcode_t ret_code, const uint32_t details_code, void * const p_callback_args);

/*
 * State variables
 */
// OSAL

osal_cond_handle_t  handle_osalcond;
osal_cond_id_t      osal_cond_id;
osal_mutex_handle_t handle_osalmutex;
osal_mutex_id_t     osal_mutex_id;

// Memory Manager

osal_memory_manager_handle_t handle_osalmmngr;
st_osal_mmngr_config_t       osal_mmngr_config;

// IMR Driver

static uintptr_t             ctrldata[MAX_MODULE][IMRDRV_SIZE_WORKAREA / sizeof(uintptr_t)];
static st_imrdrv_initdata_t  initdata[MAX_MODULE_NUM];
static imrdrv_ctrl_handle_t  ctrhandle[MAX_MODULE_NUM];
static st_imrdrv_os_config_t os_config[MAX_MODULE_NUM];
static osal_axi_bus_id_t     axi_id[MAX_MODULE_NUM];

#define OSAL_MUTEX_WAIT (1000U) /* msec */
#define TIMEOUT         (30000U) /* msec */


imrdrv_ctrl_handle_t get_imrdrv_ctrlhandles(int channel)
{
    uint32_t channel_idx = convert_channel_to_index(channel);
    return ctrhandle[channel_idx];
}

/**
 * @brief Inits OSAL layer and creates synchronization objects
 * @return
 */
int init_osal()
{
    e_osal_return_t ret_osal;
    osal_cond_id = cond_id[0];
    ret_osal     = R_OSAL_ThsyncCondCreate(osal_cond_id, &handle_osalcond);

    if (ret_osal != OSAL_RETURN_OK) {
        printf("Failed R_OSAL_ThsyncCondCreate ret=%d\n", ret_osal);
        return 1;
    }

    osal_mutex_id = callback_mutex_id[0];
    ret_osal      = R_OSAL_ThsyncMutexCreate(osal_mutex_id, &handle_osalmutex);

    if (ret_osal != OSAL_RETURN_OK) {
        printf("Failed R_OSAL_ThsyncMutexCreate ret=%d\n", ret_osal);
        return 1;
    }

    // osal_cond_id = 0x5000;      // anthony: fixme
    // ret_osal     = R_OSAL_ThsyncCondCreate(osal_cond_id, &handle_osalcond);
    // if (ret_osal != OSAL_RETURN_OK) {
    //     PRINT_ERROR("Failed R_OSAL_ThsyncCondCreate ret=%d\n", ret_osal);
    //     return 1;
    // }

    // osal_mutex_id = 0x5006;      // anthony: fixme
    // ret_osal      = R_OSAL_ThsyncMutexCreate(osal_mutex_id, &handle_osalmutex);
    // if (ret_osal != OSAL_RETURN_OK) {
    //     PRINT_ERROR("Failed R_OSAL_ThsyncMutexCreate ret=%d\n", ret_osal);
    //     return 1;
    // }

    return 0;
 }

/**
 * @brief Inits memory manager
 * @param max_mem_MiB
 * @return
 */
int init_mmgr(void)
{
    e_osal_return_t ret_osal;
    ret_osal = R_OSAL_MmngrGetOsalMaxConfig(&osal_mmngr_config);

    if (ret_osal == OSAL_RETURN_OK) 
    {
        osal_mmngr_config.mode                        = OSAL_MMNGR_ALLOC_MODE_FREE_LIST;
        //osal_mmngr_cfg.memory_limit                 = 100000000; //0x6000000; /* 96MB */
        osal_mmngr_config.max_allowed_allocations     = 256u;
        osal_mmngr_config.max_registered_monitors_cbs = 0u;
        ret_osal                                      = R_OSAL_MmngrOpen(&osal_mmngr_config, &handle_osalmmngr);
       
        if (ret_osal != OSAL_RETURN_OK)
        {
            printf("Failed R_OSAL_MmngrOpen ret=%d\n", ret_osal);
            return 1;
        }
    }
    else
    {
        printf("Failed R_OSAL_MmngrGetOsalMaxConfig ret=%d\n", ret_osal);
        return 1;
    }

    return 0;
}

/**
 * @brief Allocates IMR data
 * @param buf
 * @param imr_channel
 * @param alloc_size
 * @param phys_addr
 * @return
 */

int mmgr_helper_alloc_buffer(osal_memory_buffer_handle_t * buf, int imr_channel, uint32_t data_size, uint32_t align, addr_t * vmr_addr, uintptr_t * phys_addr)
{
    e_osal_return_t ret_osal;
    ret_osal = R_OSAL_MmngrAlloc(handle_osalmmngr, data_size, align, buf);

    if (ret_osal == OSAL_RETURN_OK)
    {
        ret_osal = R_OSAL_MmngrGetCpuPtr(*buf, (void *)vmr_addr);

        if (ret_osal != OSAL_RETURN_OK)
        {
            printf("Failed R_OSAL_MmngrGetCpuPtr ret=%d\n", ret_osal);
            return 1;
        }
    }
    else
    {
        printf("Failed R_OSAL_MmngrAlloc ret=%d\n", ret_osal);
        return 1;
    }

    ret_osal = R_OSAL_MmngrGetHwAddr(*buf, axi_id[imr_channel], phys_addr);

    if (ret_osal != OSAL_RETURN_OK)
    {
        printf("Failed R_OSAL_MmngrGetHwAddr ret=%d\n", ret_osal);
        return 1;
    }

    return 0;
}

int allocate_dl_memory(osal_memory_buffer_handle_t * mmngr_buf_handle, size_t data_size, size_t align, void * p_vmr_addr)
{
    e_osal_return_t ret_osal;
    ret_osal = R_OSAL_MmngrAlloc(handle_osalmmngr, data_size, align, mmngr_buf_handle);

    if (ret_osal == OSAL_RETURN_OK)
    {
        ret_osal = R_OSAL_MmngrGetCpuPtr(*mmngr_buf_handle, (void *)p_vmr_addr);

        if (ret_osal != OSAL_RETURN_OK)
        {
            printf("Failed R_OSAL_MmngrGetCpuPtr ret=%d\n", ret_osal);
            return 1;
        }
    }
    else
    {
        printf("Failed R_OSAL_MmngrAlloc ret=%d\n", ret_osal);
        return 1;
    }
 
    return 0;
}

int mmgr_helper_flush(osal_memory_buffer_handle_t buf)
{
    e_osal_return_t ret_osal;
    size_t          size;
    ret_osal = R_OSAL_MmngrGetSize(buf, &size);

    if (ret_osal != OSAL_RETURN_OK)
    {
        printf("Failed R_OSAL_MmngrGetSize ret=%d\n", ret_osal);
        return 1;
    }

    ret_osal = R_OSAL_MmngrFlush(buf, 0, size);

    if (ret_osal != OSAL_RETURN_OK)
    {
        printf("Failed R_OSAL_MmngrFlush ret=%d\n", ret_osal);
        return 1;
    }
    return 0;
}

int mmgr_helper_invalidate(osal_memory_buffer_handle_t buf)
{
    e_osal_return_t ret_osal;
    size_t          size;
    ret_osal = R_OSAL_MmngrGetSize(buf, &size);

    if (ret_osal != OSAL_RETURN_OK)
    {
        printf("Failed R_OSAL_MmngrGetSize ret=%d\n", ret_osal);
        return 1;
    }

    ret_osal = R_OSAL_MmngrInvalidate(buf, 0, size);

    if (ret_osal != OSAL_RETURN_OK)
    {
        printf("Failed R_OSAL_MmngrFlush ret=%d\n", ret_osal);
        return 1;
    }

    return 0;
}

/**
 * @brief Deallocate memory
 * @param buf
 * @return
 *
 * @warning  Be sure to de-allocate in exact REVERSE order than allocation (xOS 2.x limitation)
 */

int mmgr_helper_dealloc(osal_memory_buffer_handle_t buf)
{
    e_osal_return_t ret_osal;
    ret_osal = R_OSAL_MmngrDealloc(handle_osalmmngr, buf);

    if (ret_osal != OSAL_RETURN_OK)
    {
        printf("Failed R_OSAL_MmngrDealloc ret=%d\n", ret_osal);
        return 1;
    }

    return 0;
}



/**
 * @brief Inits IMR driver stuff
 * @return
 */

int init_imrdrv()
{
    e_osal_return_t      ret_osal;
    e_imrdrv_errorcode_t ret_imr;
    uint8_t              i;
    uint32_t             ch_idx;
    /* === Initialize device drivers. for ch 0 and ch 4 === */   
    ch_idx                              = convert_channel_to_index(IMR_Channel);
    initdata[ch_idx].p_work_addr        = ctrldata[ch_idx];
    initdata[ch_idx].work_size          = IMRDRV_SIZE_WORKAREA;
    initdata[ch_idx].channel_no         = channel_no[ch_idx];
    os_config[ch_idx].mutex_id          = mutex_id[ch_idx];
    os_config[ch_idx].mutex_wait_period = OSAL_MUTEX_WAIT;
    os_config[ch_idx].dev_irq_priority  = OSAL_INTERRUPT_PRIORITY_TYPE1;

    ret_imr = R_IMRDRV_Init(&initdata[ch_idx], &os_config[ch_idx], imrdrv_callback_func, (void *)&handle_osalcond, &ctrhandle[ch_idx]);

    if (ret_imr != IMRDRV_ERROR_OK)
    {
        printf("Failed R_IMRDRV_Init no=%d, ret=%d\n", channel_no[ch_idx], ret_imr);
        return 1;
    }

    printf("Succeed R_IMRDRV_Init no=%d\n", channel_no[ch_idx]);


    /* === Get axi id === */
    /* WARNING: V3U specific ! */
    ch_idx   = convert_channel_to_index(IMR_Channel);
    ret_osal = R_OSAL_IoGetAxiBusIdFromDeviceName(device_name[ch_idx], &axi_id[IMR_Channel]);

    if (ret_osal != OSAL_RETURN_OK)
    {
        printf("Failed R_OSAL_IoGetAxiBusIdFromDeviceName ret=%d\n", ret_osal);
    }

    /* === Setup IMR device driver for channel 0 and 4. === */
    ch_idx  = convert_channel_to_index(IMR_Channel);
    ret_imr = R_IMRDRV_Start(ctrhandle[ch_idx]);

    if (ret_imr != IMRDRV_ERROR_OK)
    {
        printf("Failed R_IMRDRV_Start no=%d, ret=%d\n", channel_no[ch_idx], ret_imr);
        return 1;
    }

    printf("Succeed R_IMRDRV_Start no=%d\n", channel_no[ch_idx]);

    return 0;
}

/**
 * @brief Tears down IMR driver
 * @return
 */
int deinit_imrdrv()
{
    uint8_t  i;
    uint32_t ch_idx;
    e_imrdrv_errorcode_t ret_imr;
    /* === Close IMR device driver. === */
    ret_imr = 0;
    ch_idx  = convert_channel_to_index(IMR_Channel);
    ret_imr = R_IMRDRV_Stop(ctrhandle[ch_idx]);

    if (ret_imr != IMRDRV_ERROR_OK)
    {
        PRINT_ERROR("Failed R_IMRDRV_Stop no=%d, ret=%d\n", channel_no[ch_idx], ret_imr);
        return 1;
    }

    DEBUG_PRINT("Succeed R_IMRDRV_Stop no=%d\n", channel_no[ch_idx]);
    /* === Deinitialize. === */
    ch_idx  = convert_channel_to_index(IMR_Channel);
    ret_imr = R_IMRDRV_Quit(ctrhandle[ch_idx]);

    if (ret_imr != IMRDRV_ERROR_OK)
    {
        PRINT_ERROR("Failed R_IMRDRV_Quit no=%d, ret=%d\n", channel_no[ch_idx], ret_imr);
    }
    else
    {
        DEBUG_PRINT("Succeed R_IMRDRV_Quit no=%d\n", channel_no[ch_idx]);
    }
    return 0;
}

/**
 * @brief Tears down MMGR
 * @return
 */
int deinit_mmgr()
{
    e_osal_return_t ret_osal;
    ret_osal = R_OSAL_MmngrClose(handle_osalmmngr);

    if (ret_osal != OSAL_RETURN_OK)
    {
        printf("Failed R_OSAL_MmngrClose ret=%d\n", ret_osal);
    }

    return ret_osal;
}


/**
 * @brief Waits for imrdrv callback to occur
 * @pre R_IMRDRV_Execute() called
 * @return
 *
 * WARN: It seems we can NOT simply run multiple IMR channels with current SW !
 *       Is there ANY way to find in callback WHICH imr channel finished ???
 *
 * TODO:
 */
int imrdrv_wait()
{
    e_osal_return_t ret_osal;
    ret_osal = R_OSAL_ThsyncMutexTryLock(handle_osalmutex);

    if (ret_osal != OSAL_RETURN_OK)
    {
        PRINT_ERROR("Failed R_OSAL_ThsyncMutexTryLock ret=%d\n", ret_osal);
        return 1;
    }

    ret_osal = R_OSAL_ThsyncCondWaitForTimePeriod(handle_osalcond, handle_osalmutex, (osal_milli_sec_t)TIMEOUT);

    if (ret_osal != OSAL_RETURN_OK)
    {
        if (ret_osal == OSAL_RETURN_TIME)
        {
            PRINT_ERROR("Failed(TIMEOUT) R_OSAL_ThsyncCondWaitForTimePeriod , ret=%d\n", ret_osal);
            return 2;
        }
        else
        {
            PRINT_ERROR("Failed R_OSAL_ThsyncCondWaitForTimePeriod , ret=%d\n", ret_osal);
            return 1;
        }
    }

    ret_osal = R_OSAL_ThsyncMutexUnlock(handle_osalmutex);

    if (ret_osal != OSAL_RETURN_OK)
    {
        PRINT_ERROR("Failed R_OSAL_ThsyncMutexUnlock ret=%d\n", ret_osal);
        return 1;
    }

    return 0;
}


/***************************************************************************
*   Function: imrdrv_callback_func
*
*   Description:
*       CALLBACK METHOD
*
*   Parameters:
*       ret_code             -
*       details_code         -
*       p_callback_args      -
*
*   Return:
*/
static int32_t imrdrv_callback_func(const e_imrdrv_errorcode_t ret_code, const uint32_t details_code, void * const p_callback_args)
{
    e_osal_return_t ret_osal;
    int32_t ret = 0;
    osal_cond_handle_t * p_handl_osalcond;
    p_handl_osalcond = (osal_cond_handle_t *)p_callback_args;
    (void)details_code;

    if (ret_code == IMRDRV_ERROR_OK)
    {
        ret_osal = R_OSAL_ThsyncCondSignal(* p_handl_osalcond);

        if (ret_osal != OSAL_RETURN_OK)
        {
            ret = 1;
            printf("Failed R_OSAL_ThsyncCondSignal ret=%d\n", ret_osal);
        }
        else
        {
            DEBUG_PRINT("Succeed R_OSAL_ThsyncCondSignal\n");
        }
    }
    else
    {
        printf("callback_func received error code =%d\n", ret_code);
    }

    return ret;
}



