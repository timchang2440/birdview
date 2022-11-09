/***********************************************************************************************************************
* File Name    : buffers.c
* Version      : 1.0.0
* Product Name : DMS Application
* Device(s)    : R-Car V3h2
* Description  : video buffer File
***********************************************************************************************************************/

/***********************************************************************************************************************
* History : Version DD.MM.YYYY Description
*         : 1.0.0   20.05.2022 Initial version
***********************************************************************************************************************/

/* System includes */
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
/*DRM library */
#include "include/drm.h"
#include "include/drm_fourcc.h"
#include "include/libdrm_macros.h"
#include "include/xf86drm.h"
#include "include/buffers.h"

/* Local includes */
#include "common.h"

/**
 * Configuration by defines
 */
#define VAL_ZERO            (0)
#define VAL_ONE             (1)

struct bo * bo_create_dumb(int fd, unsigned int width, unsigned int height, unsigned int bpp)
{
    struct bo * bo = {VAL_ZERO};
    struct drm_mode_create_dumb arg = {VAL_ZERO};
    int ret = INVALID;

    bo = calloc(1, sizeof(*bo));

    if (bo == NULL)
    {
        PRINT_ERROR("failed to allocate buffer object\n");
        return NULL;
    }

    memset(&arg, VAL_ZERO, sizeof(arg));
    arg.bpp    = bpp;
    arg.width  = width;
    arg.height = height;

    ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &arg);

    if (ret)
    {
        PRINT_ERROR("failed to create dumb buffer: %s\n", strerror(errno));
        free(bo);
        return NULL;
    }

    bo->fd     = fd;
    bo->handle = arg.handle;
    bo->size   = arg.size;
    bo->pitch  = arg.pitch;
    return bo;
}

int bo_map(struct bo *bo, void ** out)
{
    struct drm_mode_map_dumb arg = {VAL_ZERO};
    void * map = NULL;
    int ret    = INVALID;
    memset(&arg, VAL_ZERO, sizeof(arg));

    arg.handle = bo->handle;

    ret = drmIoctl(bo->fd, DRM_IOCTL_MODE_MAP_DUMB, &arg);

    if (ret)
    {
        PRINT_ERROR("failed drmIoctl %s\n", strerror(errno));
        return ret;
    }

    map = drm_mmap(VAL_ZERO, bo->size, PROT_READ | PROT_WRITE, MAP_SHARED, bo->fd, (__off_t)arg.offset);

    if (map == MAP_FAILED)
    {
        PRINT_ERROR("failed drm_mmap %s\n", strerror(errno));
        return -EINVAL;
    }

    bo->ptr = map;

    if (bo->ptr)
    {
        *out = map;
    }

    return SUCCESS;
}

void bo_unmap(struct bo *bo)
{
    if (!bo->ptr)
        return;

    drm_munmap(bo->ptr, bo->size);
    bo->ptr = NULL;
}

struct bo * bo_create(int fd, unsigned int format, unsigned int width, unsigned int height, unsigned int handles[4], unsigned int pitches[4], unsigned int offsets[4])
{
    struct bo *bo   = NULL;
    unsigned int bpp;
    void * virtual;
    int ret;

    switch (format) {
    case DRM_FORMAT_UYVY:
    case DRM_FORMAT_VYUY:
    case DRM_FORMAT_YUYV:
    case DRM_FORMAT_YVYU:
        bpp = 16;
        break;

    case DRM_FORMAT_BGR888:
    case DRM_FORMAT_RGB888:
        bpp = 24;
        break;

    default:
        PRINT_ERROR("unsupported format 0x%08x\n", format);
        return NULL;
    }

    bo = bo_create_dumb(fd, width, height, bpp);

    if (!bo)
        return NULL;

    ret = bo_map(bo, &virtual);

    if (ret)
    {
        PRINT_ERROR("Failed to map buffer: %s\n", strerror(-errno));
        bo_destroy(bo);
        return NULL;
    }

    switch (format)
    {
    case DRM_FORMAT_UYVY:
    case DRM_FORMAT_VYUY:
    case DRM_FORMAT_YUYV:
    case DRM_FORMAT_YVYU:
        offsets[0] = VAL_ZERO;
        handles[0] = bo->handle;
        pitches[0] = (unsigned int)bo->pitch;
        break;

    case DRM_FORMAT_BGR888:
    case DRM_FORMAT_RGB888:
        offsets[0] = VAL_ZERO;
        handles[0] = bo->handle;
        pitches[0] = (unsigned int)bo->pitch;
        break;
    }

    return bo;
}

void bo_destroy(struct bo *bo)
{
    struct drm_mode_destroy_dumb arg = {VAL_ZERO};
    int ret = INVALID;

    memset(&arg, VAL_ZERO, sizeof(arg));
    arg.handle = bo->handle;

    ret = drmIoctl(bo->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &arg);

    if (ret)
    {
        PRINT_ERROR("Failed to destroy dumb buffer: %s\n", strerror(errno));
    }

    free(bo);
}
