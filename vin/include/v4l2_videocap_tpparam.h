/*********************************************************************************************************************
* File Name    : v4l2_videocap_tpparam.c
* Version      : 1.0.0
* Product Name : DMS Application
* Device(s)    : R-Car V3h2
* Description  : Header File
*********************************************************************************************************************/

/*********************************************************************************************************************
* History : Version DD.MM.YYYY Description
*         : 1.0.0   20.05.2022 Initial version
*********************************************************************************************************************/
#ifndef __V4L2_VIDEOCAP_TPPARAM_H__
#define __V4L2_VIDEOCAP_TPPARAM_H__

#ifdef __cplusplus
extern "C"
{
#endif

struct tp_param{
    /* attribute */
    unsigned int attribute;
#define TPPARAM_ATTRIB_END              (0x00000000)
#define TPPARAM_ATTRIB_OPEN             (0x00000001)
#define TPPARAM_ATTRIB_CLOSE            (0x00000002)
#define TPPARAM_ATTRIB_MMAP             (0x00000004)
#define TPPARAM_ATTRIB_IOCTL            (0x00000008)

#define TPPARAM_ATTRIB_EXPECT_EQUAL     (0x00000000)
#define TPPARAM_ATTRIB_EXPECT_NOT_EQUAL (0x10000000)

    /* for open */
    char *dev_name;
    /* for close,mmap,ioctl */
    int fd;
    /* for mmmap */
    void *addr;
    size_t length;
    off_t offset;
    /* for ioctl */
    int request;
    void *arg;
    /* expecting value */
    int expect;
};

#define TP_RESULT_PASS               (1)
#define TP_RESULT_FAIL               (0)

/* id for tests */
#define TESTNAME_START_STOP_CAPTURE  (1)
#define TESTNAME_ERRORCASE           (255)


/* functions for tests */
extern void test_of_capture_start_stop(void);
extern void test_of_errorcase(void);

/* test parameters */

/* for v4l2_cropcap */
struct v4l2_cropcap g_cropcap_tpparam[] = {
    {
        .type = V4L2_BUF_TYPE_VIDEO_OUTPUT,
    },
    {
        .type = V4L2_BUF_TYPE_VIDEO_OVERLAY,
    },
    {
        .type = V4L2_BUF_TYPE_VBI_CAPTURE,
    },
    {
        .type = V4L2_BUF_TYPE_VBI_OUTPUT,
    },
    {
        .type = V4L2_BUF_TYPE_SLICED_VBI_CAPTURE,
    },
    {
        .type = V4L2_BUF_TYPE_SLICED_VBI_OUTPUT,
    },
    {
        .type = V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY,
    },
};

/* for v4l2_control */
struct v4l2_control g_control_tpparam[] = {
    {
        .id = V4L2_CID_GAMMA,
    },
    {
        .id = V4L2_CID_HFLIP,
    },
    {
        .id = V4L2_CID_VFLIP,
    },
};

/* for v4l2_buffer */
struct v4l2_buffer g_buf_tpparam[] = {
    {
        .type   = V4L2_BUF_TYPE_VIDEO_CAPTURE,
        .memory = V4L2_MEMORY_OVERLAY,
    },
};

/* for v4l2_requestbuffers */
struct v4l2_requestbuffers g_req_tpparam[] = {
    {
        .type   = V4L2_BUF_TYPE_VIDEO_CAPTURE,
        .memory = V4L2_MEMORY_OVERLAY,
        .count  = 4,
    },
};

enum v4l2_buf_type g_type_tpparam = V4L2_BUF_TYPE_VIDEO_OUTPUT;

/* table */
struct tp_param g_tpparam[] =
{
    /* error of open */
    {TPPARAM_ATTRIB_OPEN | TPPARAM_ATTRIB_EXPECT_NOT_EQUAL, "/dev/video" , 0, 0, 0, 0, 0, 0, -1},
    /* error of close */
    /* error of mmap */
    {TPPARAM_ATTRIB_MMAP, "/dev/video", 0, 0, 0, 0, 0, 0, -1},
    /* error of ioctl */
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_DBG_G_REGISTER, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int)VIDIOC_DBG_S_REGISTER, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int)VIDIOC_ENCODER_CMD, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int)VIDIOC_TRY_ENCODER_CMD , 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int)VIDIOC_ENUMAUDIO , 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int)VIDIOC_ENUMAUDOUT , 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int)VIDIOC_ENUM_FMT, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int)VIDIOC_ENUM_FRAMESIZES , 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int)VIDIOC_ENUM_FRAMEINTERVALS, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int)VIDIOC_ENUMINPUT , 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int)VIDIOC_ENUMOUTPUT, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int)VIDIOC_ENUMSTD , 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int)VIDIOC_G_AUDIO, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int)VIDIOC_S_AUDIO , 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_G_AUDOUT, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_S_AUDOUT , 0, -1},
    //{TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, VIDIOC_G_CHIP_IDENT , 0, -1},  // FIXME
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_G_ENC_INDEX , 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_G_EXT_CTRLS, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_S_EXT_CTRLS, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_TRY_EXT_CTRLS , 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_G_FBUF, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_S_FBUF , 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_G_FMT, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0,(int)  VIDIOC_G_FREQUENCY, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_S_FREQUENCY, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_G_INPUT, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_S_INPUT , 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_G_JPEGCOMP, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_S_JPEGCOMP, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_G_MODULATOR, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_S_MODULATOR, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_G_OUTPUT, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_S_OUTPUT, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_G_PARM, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_S_PARM , 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_G_PRIORITY, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_S_PRIORITY, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_G_SLICED_VBI_CAP , 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_G_STD, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_S_STD , 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_G_TUNER, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0,(int) VIDIOC_S_TUNER, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_LOG_STATUS, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_OVERLAY , 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_QUERYMENU, 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_QUERYSTD , 0, -1},

    /* 51 */
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_CROPCAP , 0, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_QBUF  , &g_buf_tpparam[0], -1},

    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_DQBUF  , &g_buf_tpparam[0], -1},

    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_STREAMON  , &g_type_tpparam, -1},
    {TPPARAM_ATTRIB_IOCTL, "/dev/video" , 0, 0, 0, 0, (int) VIDIOC_STREAMOFF  , &g_type_tpparam, -1},

    /* end of record */
    {TPPARAM_ATTRIB_END, NULL         , 0, 0, 0, 0, 0, 0,0}
};

#ifdef __cplusplus
}
#endif

#endif /* __V4L2_VIDEOCAP_TPPARAM_H__ */

#if 0
struct timezone{
    int tz_minutewest;
    int tz_dsttime;
};
#endif
