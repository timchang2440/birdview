/***********************************************************************************************************************
* File Name    : vout_main.c
* Version      : 1.0.0
* Product Name : DMS Application
* Device(s)    : R-Car V3h2
* Description  : Video Output Main File
***********************************************************************************************************************/

/***********************************************************************************************************************
* History : Version DD.MM.YYYY Description
*         : 1.0.0   20.05.2022 Initial version
***********************************************************************************************************************/

/* System includes */
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <poll.h>
#include <sys/time.h>
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#include <math.h>

/*DRM and util library */
#include "include/xf86drm.h"
#include "include/xf86drmMode.h"
#include "include/drm_fourcc.h"
#include "include/util/format.h"
#include "include/util/kms.h"

#include "include/buffers.h"
#include "common.h"

int vout_init (void);
int execute (void);
int64_t vout_deinit (void);

#define PLANE_ID    ( 38 )
#define CRTC_ID     ( 44 )
#define SCALE       ( 1.0 )

struct crtc
{
    drmModeCrtc * crtc;
    drmModeObjectProperties * props;
    drmModePropertyRes ** props_info;
    drmModeModeInfo * mode;
};

struct encoder
{
    drmModeEncoder * encoder;
};

struct connector
{
    drmModeConnector * connector;
    drmModeObjectProperties * props;
    drmModePropertyRes ** props_info;
    char * name;
};

struct fb
{
    drmModeFB * fb;
};

struct plane
{
    drmModePlane * plane;
    drmModeObjectProperties * props;
    drmModePropertyRes ** props_info;
};

struct resources
{
    struct crtc * crtcs;
    int count_crtcs;
    struct encoder * encoders;
    int count_encoders;
    struct connector * connectors;
    int count_connectors;
    struct fb * fbs;
    int count_fbs;
    struct plane * planes;
    uint32_t count_planes;
};

struct device
{
    int fd;

    struct resources * resources;

    struct
    {
        unsigned int width;
        unsigned int height;

        unsigned int fb_id;
        struct bo *bo;
        struct bo * cursor_bo;
    } mode;

    int use_atomic;
    drmModeAtomicReq * req;
};

struct device dev;
struct plane_arg * plane_args = NULL;
unsigned plane_count = 0;
int pixfmt_bpp;
char pix_fmt[16];

#define bit_name_fn(res)                    \
const char * res##_str(int type) {              \
    unsigned int i;                     \
    const char *sep = "";                   \
    for (i = 0; i < ARRAY_SIZE(res##_names); i++) {     \
        if (type & (1 << i)) {              \
            DEBUG_PRINT("%s%s", sep, res##_names[i]);   \
            sep = ", ";             \
        }                       \
    }                           \
    return NULL;                        \
}

static void free_resources(struct resources * res)
{
    int i;

    if (!res)
        return;

#define free_resource(_res, type, Type)                 \
    do {                                    \
        if (!(_res)->type##s)                       \
            break;                          \
        for (i = 0; i < (int)(_res)->count_##type##s; ++i) {    \
            if (!(_res)->type##s[i].type)               \
                break;                      \
            drmModeFree##Type((_res)->type##s[i].type);     \
        }                               \
        free((_res)->type##s);                      \
    } while (0)

#define free_properties(_res, type)                 \
    do {                                    \
        for (i = 0; i < (int)(_res)->count_##type##s; ++i) {    \
            unsigned int j;                                     \
            for (j = 0; j < res->type##s[i].props->count_props; ++j)\
                drmModeFreeProperty(res->type##s[i].props_info[j]);\
            free(res->type##s[i].props_info);           \
            drmModeFreeObjectProperties(res->type##s[i].props); \
        }                               \
    } while (0)

    free_properties(res, plane);
    free_resource(res, plane, Plane);

    free_properties(res, connector);
    free_properties(res, crtc);

    for (i = 0; i < res->count_connectors; i++)
        free(res->connectors[i].name);

    free_resource(res, fb, FB);
    free_resource(res, connector, Connector);
    free_resource(res, encoder, Encoder);
    free_resource(res, crtc, Crtc);

    free(res);
}

static struct resources * get_resources(struct device * dev)
{
    drmModeRes *_res;
    drmModePlaneRes * plane_res;
    struct resources * res;
    int i;

    res = calloc(1, sizeof(* res));

    if (res == 0)
        return NULL;

    drmSetClientCap(dev->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);

    _res = drmModeGetResources(dev->fd);

    if (!_res)
    {
        PRINT_ERROR("drmModeGetResources failed: %s\n", strerror(errno));
        free(res);
        return NULL;
    }

    res->count_crtcs      = _res->count_crtcs;
    res->count_encoders   = _res->count_encoders;
    res->count_connectors = _res->count_connectors;
    res->count_fbs        = _res->count_fbs;

    res->crtcs      = calloc((size_t)res->count_crtcs, sizeof(* res->crtcs));
    res->encoders   = calloc((size_t)res->count_encoders, sizeof(* res->encoders));
    res->connectors = calloc((size_t)_res->count_connectors, sizeof(* res->connectors));
    res->fbs        = calloc((size_t)res->count_fbs, sizeof(* res->fbs));

    if (!res->crtcs || !res->encoders || !res->connectors || !res->fbs)
    {
        drmModeFreeResources(_res);
        goto error;
    }

#define get_resource(_res, __res, type, Type)                   \
    do {                                    \
        for (i = 0; i < (int)(_res)->count_##type##s; ++i) {    \
            uint32_t type##id = (__res)->type##s[i];            \
            (_res)->type##s[i].type =                           \
                drmModeGet##Type(dev->fd, type##id);            \
            if (!(_res)->type##s[i].type)                       \
            PRINT_ERROR("could not get %s %i: %s\n",#type, type##id,strerror(errno));       \
        }                               \
    } while (0)

    get_resource(res, _res, crtc, Crtc);
    get_resource(res, _res, encoder, Encoder);
    get_resource(res, _res, connector, Connector);
    get_resource(res, _res, fb, FB);

    drmModeFreeResources(_res);

#define get_properties(_res, type, Type)                    \
    do {                                    \
        for (i = 0; i < (int)(_res)->count_##type##s; ++i) {    \
            struct type *obj = &res->type##s[i];            \
            unsigned int j;                     \
            obj->props =                        \
                drmModeObjectGetProperties(dev->fd, obj->type->type##_id, \
                               DRM_MODE_OBJECT_##Type); \
            if (!obj->props) {                  \
                PRINT_ERROR("could not get properties\n"); \
                continue;                   \
            }                           \
            obj->props_info = calloc(obj->props->count_props,   \
                         sizeof(*obj->props_info)); \
            if (!obj->props_info)                   \
                continue;                   \
            for (j = 0; j < obj->props->count_props; ++j)       \
                obj->props_info[j] =                \
                    drmModeGetProperty(dev->fd, obj->props->props[j]); \
        }                               \
    } while (0)

    get_properties(res, crtc, CRTC);
    get_properties(res, connector, CONNECTOR);

    for (i = 0; i < res->count_crtcs; ++i)
        res->crtcs[i].mode = &res->crtcs[i].crtc->mode;

    plane_res = drmModeGetPlaneResources(dev->fd);

    if (!plane_res)
    {
        PRINT_ERROR("drmModeGetPlaneResources failed: %s\n", strerror(errno));
        return res;
    }

    res->count_planes = plane_res->count_planes;

    res->planes = calloc(res->count_planes, sizeof(* res->planes));

    if (!res->planes)
    {
        drmModeFreePlaneResources(plane_res);
        goto error;
    }

    get_resource(res, plane_res, plane, Plane);
    drmModeFreePlaneResources(plane_res);
    get_properties(res, plane, PLANE);

    return res;

error:
    free_resources(res);
    return NULL;
}

static struct crtc * get_crtc_by_id(struct device * dev, uint32_t id)
{
    int i;

    for (i = 0; i < dev->resources->count_crtcs; ++i)
    {
        drmModeCrtc *crtc = dev->resources->crtcs[i].crtc;

        if (crtc && crtc->crtc_id == id)
            return &dev->resources->crtcs[i];
    }

    return NULL;
}

static uint32_t get_crtc_mask(struct device * dev, struct crtc * crtc)
{
    unsigned int i;

    for (i = 0; i < (unsigned int)dev->resources->count_crtcs; i++)

    {
        if (crtc->crtc->crtc_id == dev->resources->crtcs[i].crtc->crtc_id)
            return (uint32_t)(1 << i);
    }

    /* Unreachable: crtc->crtc is one of resources->crtcs[] */
    /* Don't return zero or static analysers will complain */
    abort();
    return 0;
}

struct pipe_arg
{
    const char ** cons;
    uint32_t * con_ids;
    unsigned int num_cons;
    uint32_t crtc_id;
    char mode_str[DATA_LEN_64];
    char format_str[5];
    float vrefresh;
    unsigned int fourcc;
    drmModeModeInfo * mode;
    struct crtc * crtc;
    unsigned int fb_id[2], current_fb_id;
    struct timeval start;

    int swap_count;
};

struct plane_arg
{
    uint32_t plane_id;  /* the id of plane to use */
    uint32_t crtc_id;  /* the id of CRTC to bind to */
    bool has_position;
    int32_t x, y;
    uint32_t w, h;
    double scale;
    unsigned int fb_id;
    unsigned int old_fb_id;
    struct bo *bo;
    struct bo * old_bo;
    char format_str[5]; /* need to leave room for terminating \0 */
    unsigned int fourcc;
};

struct property_arg
{
    uint32_t obj_id;
    uint32_t obj_type;
    char name[DRM_PROP_NAME_LEN+1];
    uint32_t prop_id;
    uint64_t value;
    bool optional;
};

int crtc_x, crtc_y, crtc_w, crtc_h;

int vout_exec (struct device * dev, struct plane_arg * p);

static bool set_property(struct device * dev, struct property_arg * p)
{
    drmModeObjectProperties *props = NULL;
    drmModePropertyRes **props_info = NULL;
    const char * obj_type;
    int ret;
    int i;

    p->obj_type = 0;
    p->prop_id  = 0;

#define find_object(_res, type, Type)                   \
    do {                                    \
        for (i = 0; i < (int)(_res)->count_##type##s; ++i) {    \
            struct type *obj = &(_res)->type##s[i];         \
            if (obj->type->type##_id != p->obj_id)          \
                continue;                   \
            p->obj_type = DRM_MODE_OBJECT_##Type;           \
            obj_type = #Type;                   \
            props = obj->props;                 \
            props_info = obj->props_info;               \
        }                               \
    } while(0)                              \

    find_object(dev->resources, crtc, CRTC);

    if (p->obj_type == 0)
        find_object(dev->resources, connector, CONNECTOR);

    if (p->obj_type == 0)
        find_object(dev->resources, plane, PLANE);

    if (p->obj_type == 0)
    {
        PRINT_ERROR("Object %i not found, can't set property\n",p->obj_id);
        return false;
    }

    if (!props)
    {
        PRINT_ERROR("%s %i has no properties\n",obj_type, p->obj_id);
        return false;
    }

    for (i = 0; i < (int)props->count_props; ++i)
    {
        if (!props_info[i])
            continue;

        if (strcmp(props_info[i]->name, p->name) == 0)
            break;
    }

    if (i == (int)props->count_props)
    {
        if (!p->optional)
            PRINT_ERROR("%s %i has no %s property\n", obj_type, p->obj_id, p->name);
        return false;
    }

    p->prop_id = props->props[i];

    if (!dev->use_atomic)
        ret = drmModeObjectSetProperty(dev->fd, p->obj_id, p->obj_type, p->prop_id, p->value);
    else
        ret = drmModeAtomicAddProperty(dev->req, p->obj_id, p->prop_id, p->value);

    if (ret < 0)
        PRINT_ERROR("failed to set %s %i property %s to %" PRIu64 ": %s\n", obj_type, p->obj_id, p->name, p->value, strerror(errno));

    return true;
}

static bool format_support(const drmModePlanePtr ovr, uint32_t fmt)
{
    unsigned int i;

    for (i = 0; i < ovr->count_formats; ++i)
    {
        if (ovr->formats[i] == fmt)
            return true;
    }

    return false;
}

extern void bo_unmap (struct bo *bo);
static int bo_fb_create(int fd, unsigned int fourcc, const uint32_t w, const uint32_t h, struct bo ** out_bo, unsigned int * out_fb_id)
{
    uint32_t handles[4] = {0}, pitches[4] = {0}, offsets[4] = {0};
    struct bo *bo;

    unsigned int fb_id;

    bo = bo_create(fd, fourcc, w, h, handles, pitches, offsets);

    if (bo == NULL)
        return -1;

    if (drmModeAddFB2(fd, w, h, fourcc, handles, pitches, offsets, &fb_id, 0))
    {
        PRINT_ERROR("failed to add fb (%ux%u): %s\n", w, h, strerror(errno));
        bo_destroy(bo);
        return -1;
    }

    *out_bo = bo;
    *out_fb_id = fb_id;

    return 0;
}

static int set_plane(struct device * dev, struct plane_arg * p)
{
    drmModePlane * ovr;
    uint32_t plane_id;
    struct crtc * crtc = NULL;
    unsigned int i, crtc_mask;

    /* Find an unused plane which can be connected to our CRTC. Find the
     * CRTC index first, then iterate over available planes.
     */
    crtc = get_crtc_by_id(dev, p->crtc_id);

    if (!crtc)
    {
        PRINT_ERROR("CRTC %u not found\n", p->crtc_id);
        return -1;
    }

    crtc_mask = get_crtc_mask(dev, crtc);
    plane_id  = p->plane_id;

    for (i = 0; i < dev->resources->count_planes; i++)
    {
        ovr = dev->resources->planes[i].plane;

        if (!ovr)
            continue;

        if (plane_id && plane_id != ovr->plane_id)
            continue;

        if (!format_support(ovr, p->fourcc))
            continue;

        if ((ovr->possible_crtcs & crtc_mask) &&
            (ovr->crtc_id == 0 || ovr->crtc_id == p->crtc_id)) {
            plane_id = ovr->plane_id;
            break;
        }
    }

    if (i == dev->resources->count_planes)
    {
        PRINT_ERROR("no unused plane available for CRTC %u\n", p->crtc_id);
        return -1;
    }

    DEBUG_PRINT("Display %dx%d@%s on plane %u\n",
        p->w, p->h, p->format_str, plane_id);
    /* just use single plane format for now.. */
    if (bo_fb_create(dev->fd, p->fourcc, p->w, p->h, &p->bo, &p->fb_id))
        return -1;

    crtc_w = (int)(p->w * p->scale);
    crtc_h = (int)(p->h * p->scale);

    if (!p->has_position)
    {
        /* Default to the middle of the screen */
        crtc_x = (crtc->mode->hdisplay - crtc_w) / 2;
        crtc_y = (crtc->mode->vdisplay - crtc_h) / 2;
    } else
    {
        crtc_x = p->x;
        crtc_y = p->y;
    }

    ovr->crtc_id = p->crtc_id;

    return 0;
}

int vout_exec(struct device * dev, struct plane_arg * p)

{
    memcpy(p->bo->ptr, (char *)vout_in, (size_t) (g_frame_width * g_frame_height * pixfmt_bpp));

    /* note src coords (last 4 args) are in Q16 format */
    if (drmModeSetPlane(dev->fd, p->plane_id, p->crtc_id, p->fb_id, 0, crtc_x, crtc_y, (uint32_t)crtc_w, (uint32_t)crtc_h, 0, 0, p->w << 16, p->h << 16))
    {
        PRINT_ERROR("failed to enable plane: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

static void clear_planes(struct device * dev, struct plane_arg * p)
{

    if (p->fb_id)
    {
        drmModeRmFB(dev->fd, p->fb_id);
    }

    if (p->bo)
    {
        bo_destroy(p->bo);
    }
}

#define VOUT_Pos_X  0
#define VOUT_Pos_Y  0
static int set_plane_properties(struct plane_arg * plane)
{
    plane->plane_id     = PLANE_ID;
    plane->crtc_id      = CRTC_ID;
    plane->w            = g_frame_width;
    plane->h            = g_frame_height;
    plane->x            = VOUT_Pos_X;
    plane->y            = VOUT_Pos_Y;
    plane->has_position = true;
    plane->scale        = SCALE;
    strcpy(plane->format_str, pix_fmt);

    plane->fourcc = util_format_fourcc(plane->format_str);

    if (plane->fourcc == 0)
    {
        PRINT_ERROR("unknown format %s\n", plane->format_str);
        return -EINVAL;
    }

    return 0;
}

#define VOUT_Display_Format 0
int vout_init(void)
{
    int test_vsync          = 0;
    int set_preferred       = 0;
    char * device           = NULL;
    unsigned int i          = 0;
    unsigned int count      = 0;
    unsigned int prop_count = 0;
    char module[16];

    strcpy(module, "rcar-du");
    struct property_arg * prop_args = NULL;
    int ret = 0;
    memset(&dev, 0, sizeof dev);

    switch(VOUT_Display_Format)
    {
        case 0:
            pixfmt_bpp=2;
            strcpy(pix_fmt,"YUYV");
            break;

        case 1:
            pixfmt_bpp=2;
            strcpy(pix_fmt,"UYVY");
            break;

        case 2:
            pixfmt_bpp=3;
            strcpy(pix_fmt,"BG24");
            break;

        default:
            pixfmt_bpp=2;
            strcpy(pix_fmt,"YUYV");
            break;
    }

    plane_args = realloc(plane_args, (plane_count + 1) * sizeof *plane_args);

    if (plane_args == NULL)
    {
        PRINT_ERROR("memory allocation failed. \n");
        return 1;
    }

    memset(&plane_args[plane_count], 0, sizeof(* plane_args));

    set_plane_properties(&plane_args[plane_count]);
    plane_count++;
    /* Dump all the details when no* arguments are provided. */

    if (test_vsync && !count)
    {
        PRINT_ERROR("page flipping requires at least one -s option.\n");
        return -1;
    }

    if (set_preferred && count)
    {
        PRINT_ERROR("cannot use -r (preferred) when -s (mode) is set\n");
        return -1;
    }

    if (set_preferred && plane_count)
    {
        PRINT_ERROR("cannot use -r (preferred) when -P (plane) is set\n");
        return -1;
    }

    dev.fd = util_open(device, module);
    if (dev.fd < 0)
        return -1;

    dev.resources = get_resources(&dev);
    if (!dev.resources) {
        drmClose(dev.fd);
        return 1;
    }

    for (i = 0; i < prop_count; ++i)
        set_property(&dev, &prop_args[i]);

    if (set_preferred || count || plane_count)
    {
        uint64_t cap = 0;

        ret = drmGetCap(dev.fd, DRM_CAP_DUMB_BUFFER, &cap);

        if (ret || cap == 0)
        {
            PRINT_ERROR("driver doesn't support the dumb buffer API\n");
            return 1;
        }

    }

    if (set_plane(&dev, plane_args))
    {
        return -1;
    }

    return SUCCESS;
}

int64_t vout_deinit(void)
{
    clear_planes(&dev, plane_args);
    free_resources(dev.resources);
    drmClose(dev.fd);
    return 0;
}

int execute(void)
{
    if (vout_exec(&dev, plane_args))
    {
        return -1;
    }

    return SUCCESS;
}
