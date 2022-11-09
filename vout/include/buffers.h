/***********************************************************************************************************************
* File Name    : buffers.h
* Version      : 1.0.0
* Product Name : DMS Application
* Device(s)    : R-Car V3h2
* Description  : header File
***********************************************************************************************************************/

/***********************************************************************************************************************
* History : Version DD.MM.YYYY Description
*         : 1.0.0   20.05.2022 Initial version 
***********************************************************************************************************************/

#ifndef __BUFFERS_H__
#define __BUFFERS_H__


struct bo
{
    int fd;
    void * ptr;
    size_t size;
    size_t offset;
    size_t pitch;
    unsigned handle;
};

struct bo * bo_create (int fd, unsigned int format, unsigned int width, unsigned int height, unsigned int handles[4], unsigned int pitches[4], unsigned int offsets[4]);

void bo_destroy (struct bo * bo);
#endif
