
/**
 * Helper functions (inofficial) to simplify OSAL/MMGR/IMR Driver usage
 */

#ifndef OSALDRV_HELPER_H
#define OSALDRV_HELPER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "rcar-xos/imr/r_imrdrv_type.h"
#include "imrlxsample_syn.h"

int R_IMR_ExecuteLDC(void);
int R_IMR_ExecuteResize(void);
int init_osal ();
int init_mmgr ();
int init_imrdrv ();
int deinit_imrdrv ();
int deinit_mmgr ();
int mmgr_helper_alloc_buffer (osal_memory_buffer_handle_t * buf, int imr_channel, uint32_t data_size, uint32_t align, addr_t * vmr_addr, uintptr_t * phys_addr);
int mmgr_helper_flush (osal_memory_buffer_handle_t buf);
int mmgr_helper_invalidate (osal_memory_buffer_handle_t buf);
int mmgr_helper_dealloc (osal_memory_buffer_handle_t buf);
int allocate_dl_memory (osal_memory_buffer_handle_t * mmngr_buf_handle, size_t data_size, size_t align, void * p_vmr_addr);

/**
 * @brief Returns  imrdrv ctrl handles
 * @see init_imrdrv
 *
 * This function returns pointer to list of IMR channel control handles (array with MAX_MODULE elements),
 * which is created by init_imrdrv() and needed to communicate with the IMR driver for a specific channel.
 */
imrdrv_ctrl_handle_t get_imrdrv_ctrlhandles(int channel);
/**
 * @brief Waits for IMR driver callback
 * @return 0 on success
 * NOTE: Won't work when running DLs on multiple IMRs (we may have a design issue here)
 */
int imrdrv_wait ();

#ifdef __cplusplus
}
#endif

#endif // OSALDRV_HELPER_H
