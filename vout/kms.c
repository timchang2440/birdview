/***********************************************************************************************************************
* File Name    : kms.c
* Version      : 1.0.0
* Product Name : DMS Application
* Device(s)    : R-Car V3h2
* Description  : kms File
***********************************************************************************************************************/

/***********************************************************************************************************************
* History : Version DD.MM.YYYY Description
*         : 1.0.0   20.05.2022 Initial version
***********************************************************************************************************************/
/* System includes */
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*DRM library */
#include "include/xf86drm.h"
//#include "xf86drmMode.h"
#include "common.h"

/**
 * Configuration by defines
 */

int util_open(const char * device, const char * module)
{
    int fd = INVALID;

    if (module)
    {
        fd = drmOpen(module, device);

        if (fd < 0)
        {
            PRINT_ERROR("Failed to open device '%s': %s\n", module, strerror(errno));
            return -errno;
        }

    }
    else
    {
        PRINT_ERROR("No device found \n");
        return -ENODEV;
    }

    return fd;
}
