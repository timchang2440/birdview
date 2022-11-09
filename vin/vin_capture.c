/*********************************************************************************************************************
* File Name    : vin_capture.c
* Version      : 1.0.0
* Product Name : DMS Application
* Device(s)    : R-Car V3h2
* Description  : video capture
*********************************************************************************************************************/

/*********************************************************************************************************************
* History : Version DD.MM.YYYY Description
*         : 1.0.0   20.05.2022 Initial version
*********************************************************************************************************************/

/**
 *System includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>                             /* getopt_long() */
#include <fcntl.h>                              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>                          /* for videodev2.h */
#include <linux/videodev2.h>

/**
 *Local includes
 */

#include "include/vin_capture.h"
#include "common.h"
#include "include/v4l2_videocap_tpparam.h"              /* test parameters */
#include "customize.h"


//#define VIN_SAVE_FILE
#ifdef VIN_SAVE_FILE
FILE *VIN_FD = NULL;
#endif  // VIN_SAVE_FILE

/**
 *private macros
 */

#define GEN3            (1)
#define REQ_BUF_CNT     (4)
#define FILENAME_LEN    (150)
#define DEVICE_NAME     "/dev/video0"
#define VAL_ZERO        (0)
#define VAL_ONE         (1)

#define CLEAR(x) memset (&(x), 0, sizeof (x))
#define ALIGN(x, a)     (((x) + (a) - 1) & ~((a) - 1))

/* enum for io method */
typedef enum
{
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
    IO_METHOD_DMABUF,
} io_method;

struct buffer
{
    void * start;
    size_t length;
    int    dbuf_fd;
    int    id;
};

struct stream
{
    int v4lfd;
    int current_buffer;
    int buffer_count;
    struct buffer *buffer;
} stream;

/**
 * Static Variables
 */

struct buffer *     buffers        = NULL;
static char         dev_name[12];
static char *       subdev_name    = NULL;
static io_method    io             = IO_METHOD_MMAP;
static int          fd             = INVALID;
static int          subdev_fd      = INVALID;
static int          frame          = INVALID;
static int          ofsx_c         = VAL_ZERO;
static int          ofsy_c         = VAL_ZERO;
static int          width_c        = VAL_ZERO;
static int          height_c       = VAL_ZERO;
static int          width_s        = VAL_ZERO;
static int          height_s       = VAL_ZERO;
static int          cropcap_width  = VAL_ZERO;
static int          cropcap_height = VAL_ZERO;
static int          in_width       = VAL_ZERO;
static int          in_height      = VAL_ZERO;
static int          InputIndex     = INVALID;
static int          trans          = REQ_BUF_CNT;
static int          video_ch       = VAL_ZERO;
static int          LapTime        = VAL_ZERO;
static int          Fps            = VAL_ZERO;
static unsigned int n_buffers      = VAL_ZERO;
static unsigned int pixelformat    = V4L2_PIX_FMT_YUYV;
static unsigned int field          = V4L2_FIELD_NONE;
static float        bpp            = VAL_ZERO;

/**
 * local Variables
 */

unsigned long StartTime;
unsigned long FinishTime;
unsigned long Diff;

/**
 * Static Function Declaration
 */

static int R_VIN_InitParams ();
static int R_VIN_Stop_Capturing (void);
static int R_VIN_Start_Capturing ();
static int R_VIN_Uninit_Device (void);
static int R_Init_Read (unsigned int buffer_size);
static int R_VIN_Init_mmap (void);
static int R_VIN_Get_Capture_Size (void);

/**
 * local Functions
 */

int           R_VIN_ReadFrame (void);
int           R_VIN_Mainloop ();

/**************************************************************************************
 * start of function xioctl()
 * ************************************************************************************/

static int xioctl(int fdIn, int request, void * arg)
{
    int r;
    do r = ioctl(fdIn, (long unsigned int)request, arg);
    while (-1 == r && EINTR == errno);

    return r;
}

/**************************************************************************************
 * start of function R_VIN_ReadFrame()
 * ************************************************************************************/

int R_VIN_ReadFrame(void)
{
    struct v4l2_buffer buf;
    int srcsize = 0;
    int ret = 0;
    //int bpp = 0;
    //bpp = BPP_YUV;  //BPP_RGB;

    srcsize = (in_width * height_s) * bpp;

    CLEAR(buf);

    buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(fd, (int)VIDIOC_DQBUF, &buf))
    {
        switch (errno)
        {
        case EAGAIN:
            return SUCCESS;

        default:
            PRINT_ERROR("VIDIOC_DQBUF error %d, %s\n", errno, strerror(errno));
            return FAILED;
        }
    }

    assert(buf.index < n_buffers);

    DEBUG_PRINT("srcsize %u\n", srcsize);
    /* copy data to VIN destination buffer */;
    memcpy(vin_out_buffer, buffers[buf.index].start, (size_t)srcsize);
    //vin_out_buffer_update = true;
/*
        FinishTime = GetTimer();
        Diff       = (FinishTime - StartTime);
        printf("VIN srcsize:       %d\t%dms\n", srcsize, Diff);
        StartTime = GetTimer();
*/
  #ifdef VIN_SAVE_FILE
    ssize_t  bytes;
	bytes = fwrite (buffers[buf.index].start, sizeof(uint8_t), srcsize, VIN_FD);
	fflush(VIN_FD);
  #endif    // VIN_SAVE_FILE

    ret = xioctl(fd, (int)VIDIOC_QBUF, &buf);
    if (-1 == ret)
    {
        PRINT_ERROR("VIDIOC_QBUF error %d, %s\n", errno, strerror(errno));
        return FAILED;
    }


    return SUCCESS;
}

//#define VIN_FPS

unsigned int time_total = VAL_ZERO;

/**************************************************************************************
 * start of function R_VIN_Mainloop()
 * * ***********************************************************************************/
int R_VIN_Mainloop()
{
    unsigned int wait_count = VAL_ZERO;
    unsigned int count      = (unsigned int)frame;

    fd_set fds;
    struct timeval tv;
    int ret;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

  #ifdef VIN_FPS
    LapTime = VAL_ONE;

    if (LapTime == VAL_ONE)
    {
        StartTime = GetTimer();
    }
  #endif

    /* Timeout. */
    tv.tv_sec  = VAL_ONE;
    tv.tv_usec = VAL_ZERO;

    ret = select(fd + 1, &fds, NULL, NULL, &tv);

    if (INVALID == ret)
    {
        PRINT_ERROR("file discriptor select error \n");
        return FAILED;
    }

    if (VAL_ZERO == ret)
    {
        fprintf(stderr, "select timeout\n");
        return FAILED;
    }

    if (SUCCESS == R_VIN_ReadFrame())
    {
      #ifdef VIN_FPS
        if (LapTime == VAL_ONE)
        {
            /*
            if (frame < 1000)
            {
                DEBUG_PRINT("Specify frame count >= 1000\n");
                return FAILED;
            }
            */

            FinishTime = GetTimer();
            Diff       = (FinishTime - StartTime);

            Fps++;
            time_total += (unsigned int)Diff;
            //DEBUG_PRINT("time_total: %u\n", time_total);

            if (field == V4L2_FIELD_NONE)
                wait_count = 60;
            else
                wait_count = 30;

            if (time_total / 1000 >= 1)
            {
                printf("DEBUG: %s(%d): <%d fps>\n", __func__, __LINE__, Fps);
                //DEBUG_PRINT("<%dfps>\n", Fps);
                Fps        = 0;
                time_total = 0;
            }
        }
      #endif
    }
    else{

        PRINT_ERROR("Failed R_VIN_ReadFrame \n");
        return FAILED;
    }

    return SUCCESS;
}

/**************************************************************************************
 * start of function R_VIN_Stop_Capturing()
 * ************************************************************************************/

static int R_VIN_Stop_Capturing(void)
{
    enum v4l2_buf_type type;
    int ret = 0;

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ret = xioctl(fd, VIDIOC_STREAMOFF, &type);
    if (-1 == ret)
    {
        PRINT_ERROR("VIDIOC_STREAMOFF error %d, %s\n", errno, strerror(errno));
        return FAILED;
    }
    return SUCCESS;
}

/**************************************************************************************
 * start of function R_VIN_Start_Capturing()
 * * ***********************************************************************************/

static int R_VIN_Start_Capturing()
{
    unsigned int i;
    int ret = 0;
    enum v4l2_buf_type type;

    for (i = 0; i < n_buffers; ++i)
    {
        struct v4l2_buffer buf;

        CLEAR(buf);

        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index  = i;

        ret = xioctl(fd, (int)VIDIOC_QBUF, &buf);
        if (-1 == ret)
        {
            PRINT_ERROR("VIDIOC_QBUF error %d, %s\n", errno, strerror(errno));
            return FAILED;
        }
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ret = xioctl(fd, VIDIOC_STREAMON, &type);
    if (-1 == ret)
    {
        PRINT_ERROR("VIDIOC_STREAMON error %d, %s\n", errno, strerror(errno));
        return FAILED;
    }

    return SUCCESS;
}

/**************************************************************************************
 * start of function R_VIN_Uninit_Device()
 * ***********************************************************************************/

static int R_VIN_Uninit_Device(void)
{
    unsigned int i;
    int ret;

    struct v4l2_requestbuffers req;

    CLEAR(req);

    switch (io)
    {
    case IO_METHOD_READ:
        free(buffers[0].start);
        break;

    case IO_METHOD_MMAP:
        req.memory = V4L2_MEMORY_MMAP;

        for (i = 0; i < n_buffers; ++i)
        {
            ret = munmap(buffers[i].start, buffers[i].length);
            if (-1 == ret)
            {
                PRINT_ERROR("munmap error %d, %s\n", errno, strerror(errno));
                return FAILED;
            }
        }
        break;

    default:
        /* Do Nothing*/
        break;
    }

    free(buffers);

    xioctl(fd, VIDIOC_LOG_STATUS, "VIDIOC_LOG_STATUS");

    req.count = 0;
    req.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 == xioctl(fd, (int)VIDIOC_REQBUFS, &req))
    {

        if (EINVAL == errno)
        {
            fprintf(stderr, "%s does not support memory mapping\n", dev_name);
            return FAILED;
        }
        else
        {
            PRINT_ERROR("VIDIOC_REQBUFS error %d, %s\n", errno, strerror(errno));
            return FAILED;
        }
    }
    return SUCCESS;

}

/**************************************************************************************
 * start of function R_Init_Read()
 * ***********************************************************************************/

static int R_Init_Read(unsigned int buffer_size)
{
    buffers = calloc(1, sizeof(* buffers));

    if (!buffers)
    {
        fprintf(stderr, "Out of memory\n");
        return FAILED;
    }

    buffers[0].length = buffer_size;
    buffers[0].start  = malloc(buffer_size);

    if (!buffers[0].start)
    {
        fprintf(stderr, "Out of memory\n");
        return FAILED;
    }
    return SUCCESS;
}

/**************************************************************************************
 * start of function R_VIN_Init_mmap()
 * * ***********************************************************************************/

static int R_VIN_Init_mmap(void)
{
    struct v4l2_requestbuffers req;

    CLEAR(req);

    req.count  = (__u32)trans;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(fd, (int)VIDIOC_REQBUFS, &req))
    {

        if (EINVAL == errno)
        {
            fprintf(stderr, "%s does not support memory mapping\n", dev_name);
            return FAILED;
        }
        else
        {
            PRINT_ERROR("VIDIOC_REQBUFS error %d, %s\n", errno, strerror(errno));
            return FAILED;
        }
    }

    if (req.count < 2)
    {
        fprintf(stderr, "Insufficient buffer memory on %s\n", dev_name);
        return FAILED;
    }

    buffers = calloc(req.count, sizeof(* buffers));

    if (!buffers)
    {
        fprintf(stderr, "Out of memory\n");
        return FAILED;
    }

    for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
    {
        struct v4l2_buffer buf;

        CLEAR(buf);

        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index  = n_buffers;

        if (INVALID == xioctl(fd, (int)VIDIOC_QUERYBUF, &buf))
        {
            PRINT_ERROR("VIDIOC_QUERYBUF error %d, %s\n", errno, strerror(errno));
            return FAILED;
        }

        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start  = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

        if (MAP_FAILED == buffers[n_buffers].start)
        {
            PRINT_ERROR("mmap error %d, %s\n", errno, strerror(errno));
            return FAILED;
        }
    }
    return SUCCESS;
}

/**************************************************************************************
 * start of function R_VIN_SetUpVideoInput()
 * * *********************************************************************************/
int R_VIN_SetUpVideoInput()
{
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    struct v4l2_input input;

    unsigned int min;
    int ret;

    if (INVALID == xioctl(fd, (int)VIDIOC_QUERYCAP, &cap))
    {

        if (EINVAL == errno)
        {
            PRINT_ERROR("%s is no V4L2 device\n", dev_name);

            return INVALID;
        }
        else
        {
            PRINT_ERROR("VIDIOC_QUERYCAP error %d, %s\n", errno, strerror(errno));
            return INVALID;
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        PRINT_ERROR("%s is no video capture device\n", dev_name);

        return INVALID;
    }

    switch (io)
    {
    case IO_METHOD_READ:

        if (!(cap.capabilities & V4L2_CAP_READWRITE))
        {
            PRINT_ERROR("%s does not support read i/o\n", dev_name);
        }

        break;

    case IO_METHOD_MMAP:
    case IO_METHOD_USERPTR:
    case IO_METHOD_DMABUF:

        if (!(cap.capabilities & V4L2_CAP_STREAMING))
        {
            PRINT_ERROR("%s does not support streaming i/o\n", dev_name);
            return INVALID;
        }

        break;
    }

    if (InputIndex >= VAL_ZERO)
    {
        input.index = (__u32)InputIndex;
        DEBUG_PRINT("input.index:%d\n", input.index);
        DEBUG_PRINT("<VIDIOC_ENUMINPUT>\n");

        if (-1 == xioctl(fd, (int)VIDIOC_ENUMINPUT, &input))
        {
            fprintf(stderr, "%d set input number error\n", input.index);
            return FAILED;
        }

        DEBUG_PRINT("</VIDIOC_ENUMINPUT>\n");

        DEBUG_PRINT("<VIDIOC_S_INPUT>\n");

        if (-1 == xioctl(fd, (int)VIDIOC_S_INPUT, &input))
        {
            fprintf(stderr, "error!! set input number[%d]\n", input.index);
            return FAILED;
        }

        DEBUG_PRINT("</VIDIOC_S_INPUT>\n");
    }

    /* Select video input, video standard and tune here. */
    DEBUG_PRINT("<VIDIOC_S_CROP>\n");
    CLEAR(cropcap);

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (VAL_ZERO == xioctl(fd, (int)VIDIOC_CROPCAP, &cropcap))
    {
        crop.c = cropcap.defrect; /* reset to default */
        DEBUG_PRINT("Capacity(%d,%d)(%dx%d)\n", crop.c.left, crop.c.top, crop.c.width, crop.c.height);

        cropcap_width  = (int)crop.c.width;
        cropcap_height = (int)crop.c.height;

        if ((width_c > 0) && (height_c > 0))
        {
            crop.c.width  = (__u32)width_c;
            crop.c.height = (__u32)height_c;
        }

        if ((ofsx_c >= 0) && (ofsy_c >= 0))
        {
            crop.c.left = ofsx_c;
            crop.c.top  = ofsy_c;
        }

        DEBUG_PRINT("Set Crop (%d, %d) (%dx%d)\n", crop.c.left, crop.c.top, crop.c.width, crop.c.height);
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    }
    else
    {
        DEBUG_PRINT("<Errors ignored.>\n");
        /* Errors ignored. */
    }

    DEBUG_PRINT("</VIDIOC_S_CROP>\n");
    DEBUG_PRINT("<VIDIOC_TRY_FMT>\n");
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = (__u32)width_s;
    fmt.fmt.pix.height      = (__u32)height_s;
    fmt.fmt.pix.pixelformat = pixelformat;
    fmt.fmt.pix.field       = V4L2_FIELD_ANY;

    if (0 == xioctl(fd, (int)VIDIOC_TRY_FMT, &fmt))
    {
        DEBUG_PRINT("<(%dx%d)>\n", fmt.fmt.pix.width, fmt.fmt.pix.height);
        DEBUG_PRINT("<(pixel:0x%x, field:0x%x)>\n", fmt.fmt.pix.pixelformat, fmt.fmt.pix.field);
    }

    DEBUG_PRINT("</VIDIOC_TRY_FMT>\n");
    DEBUG_PRINT("<VIDIOC_S_FMT>\n");
    CLEAR(fmt);

    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = (__u32)width_s;
    fmt.fmt.pix.height      = (__u32)height_s;
    fmt.fmt.pix.pixelformat = (__u32)pixelformat;
    fmt.fmt.pix.field       = (__u32)field;

    if (INVALID == xioctl(fd, (int)VIDIOC_S_FMT, &fmt))
    {
        PRINT_ERROR("VIDIOC_S_FMT error %d, %s\n", errno, strerror(errno));
        return FAILED;
    }

    /* Note VIDIOC_S_FMT may change width and height. */

    /* Buggy driver paranoia. */
    min = fmt.fmt.pix.width * 2;

    if (fmt.fmt.pix.bytesperline < min)
        fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;

    if (fmt.fmt.pix.sizeimage < min)
        fmt.fmt.pix.sizeimage = min;

    if (width_s > (int)fmt.fmt.pix.width)
        width_s = (int)fmt.fmt.pix.width;

    if (height_s > (int)fmt.fmt.pix.height)
        height_s = (int)fmt.fmt.pix.height;

    DEBUG_PRINT("</VIDIOC_S_FMT>\n");

    switch (io)
    {
    case IO_METHOD_READ:
        R_Init_Read(fmt.fmt.pix.sizeimage);
        break;

    case IO_METHOD_MMAP:
        ret = R_VIN_Init_mmap();
        if (FAILED == ret)
        {
            return FAILED;
        }
        break;

    default:
        break;

    }

    return SUCCESS;
}

/**************************************************************************************
 * start of function R_VIN_Get_Capture_Size()
 * * *********************************************************************************/
static int R_VIN_Get_Capture_Size(void)
{

    struct v4l2_crop   crop;
    struct v4l2_format fmt;

    /* Select video input, video standard and tune here. */
    DEBUG_PRINT("<VIDIOC_G_CROP>\n");
    CLEAR(crop);

    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(fd, (int)VIDIOC_G_CROP, &crop);

    DEBUG_PRINT("<G_CROP: size (%dx%d), position (%d,%d)>\n", crop.c.width, crop.c.height, crop.c.left, crop.c.top);

    DEBUG_PRINT("</VIDIOC_G_CROP>\n");
    DEBUG_PRINT("<VIDIOC_G_FMT>\n");
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (INVALID == xioctl(fd, (int)VIDIOC_G_FMT, &fmt))
    {
        PRINT_ERROR("VIDIOC_G_FMT error %d, %s\n", errno, strerror(errno));
        return FAILED;
    }

    DEBUG_PRINT("<G_FMT: size (%dx%d), field:%d, format:%d>\n", fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.field, fmt.fmt.pix.pixelformat);

    in_width  = (int)fmt.fmt.pix.width;
    in_height = (int)fmt.fmt.pix.height;

    DEBUG_PRINT("</VIDIOC_G_FMT>\n");
    return SUCCESS;
}

/*******************************************************************************************
 * start of function R_VIN_Close()
 * *****************************************************************************************/
int R_VIN_Close(void)
{
    if (INVALID == close(fd))
    {
        PRINT_ERROR("close error %d, %s\n", errno, strerror(errno));
    }

    fd = INVALID;

    return SUCCESS;
}

/*******************************************************************************************
 * start of function R_VIN_Open()
 * *****************************************************************************************/
int R_VIN_Open()
{
    struct stat st;

    if (INVALID == stat(dev_name, &st))
    {
        PRINT_ERROR("Cannot identify '%s': %d, %s\n", dev_name, errno, strerror(errno));
        return INVALID;
    }

    if (!S_ISCHR(st.st_mode))
    {
        PRINT_ERROR("%s is no device\n", dev_name);
        return INVALID;
    }

    fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
    if (INVALID == fd)
    {
        PRINT_ERROR("Cannot open '%s': %d, %s\n", dev_name, errno, strerror(errno));
    }

    if (subdev_name)
    {
        subdev_fd = open(subdev_name, O_RDWR | O_NONBLOCK, 0);

        if (INVALID == subdev_fd)
        {
            PRINT_ERROR("Cannot open '%s': %d, %s\n", subdev_name, errno, strerror(errno));
            return INVALID;
        }
    }

    return SUCCESS;
}

/*******************************************************************************************
 * start of function R_VIN_Initialize()
 * *****************************************************************************************/

int R_VIN_Initialize(int devId) {

    int ret = FAILED;

    R_VIN_InitParams();
    ret = R_VIN_Open();
    if (SUCCESS != ret)
    {
        PRINT_ERROR("R_VIN_Open failed\n")
        return FAILED;
    }
    ret = R_VIN_SetUpVideoInput();
    if (SUCCESS != ret)
    {
        PRINT_ERROR("R_VIN_Open failed\n");
        return FAILED;
    }
    ret = R_VIN_Get_Capture_Size();
    if (SUCCESS != ret)
    {
        PRINT_ERROR("R_VIN_Open failed\n");
        return FAILED;
    }

    ret = R_VIN_Start_Capturing();
    if (SUCCESS != ret)
    {
        PRINT_ERROR("R_VIN_Open failed\n");
        return FAILED;
    }

  #ifdef VIN_SAVE_FILE
    char r[50] = {};
	sprintf(r, "%s", "/home/root/VIN.yuv");
	remove(r);
	if (NULL == (VIN_FD = fopen(r, "wb"))) {
        printf("open vin save file failed\n");
    }
  #endif    // VIN_SAVE_FILE

    return SUCCESS;
}

/*******************************************************************************************
 * start of function R_VIN_Execute()
 * *****************************************************************************************/

int R_VIN_Execute()
{
    int ret = R_VIN_Mainloop();

    if (FAILED == ret)
    {
        PRINT_ERROR("Failed R_VIN_Mainloop \n");
    }

    return ret;
}

/*******************************************************************************************
 * start of function R_VIN_DeInitialize()
 * *****************************************************************************************/

int R_VIN_DeInitialize()
{
    int ret;
    ret = R_VIN_Stop_Capturing();
    if (FAILED == ret)
    {
        PRINT_ERROR("Failed R_VIN_Stop_Capturing \n");
    }
    ret = R_VIN_Uninit_Device();
    if (FAILED == ret)
    {
        PRINT_ERROR("Failed R_VIN_Uninit_Device \n");
    }
    R_VIN_Close();

  #ifdef VIN_SAVE_FILE
    fclose(VIN_FD);
  #endif    // VIN_SAVE_FILE

    return SUCCESS;
}

/*******************************************************************************************
 * start of function R_VIN_InitParams()
 * *****************************************************************************************/

static int R_VIN_InitParams()
{
    video_ch    = 0;
    sprintf(dev_name,"/dev/video%d", video_ch);
    width_c     = 1280;
    height_c    = 800;
    width_s     = 1280;
    height_s    = 800;
    ofsx_c      = 0;
    ofsy_c      = 0;
    #ifdef NV16
    pixelformat = V4L2_PIX_FMT_NV16; /*V4L2_PIX_FMT_YUYV;  V4L2_PIX_FMT_NV16*/
    #else
    pixelformat = V4L2_PIX_FMT_YUYV; /*V4L2_PIX_FMT_YUYV;  V4L2_PIX_FMT_NV16*/
    #endif
    if (pixelformat == V4L2_PIX_FMT_NV12)
        bpp = 1.5;
    else
        bpp = 2;


    return SUCCESS;
}
