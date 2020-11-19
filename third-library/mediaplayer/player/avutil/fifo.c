/*
 * a very simple circular buffer FIFO implementation
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard
 * Copyright (c) 2006 Roman Shaposhnik
 *
 * This file is part of player.
 *
 * player is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "fifo.h"
#include "utils.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static FifoBuffer *fifo_alloc_common(void *buffer, size_t size)
{
    FifoBuffer *f;
    if (!buffer)
        return NULL;
    f = calloc(1, sizeof(FifoBuffer));
    if (!f) {
        free(buffer);
        return NULL;
    }
    f->buffer = buffer;
    f->end    = f->buffer + size;
    av_fifo_reset(f);
    return f;
}

FifoBuffer *av_fifo_alloc(unsigned int size)
{
    void *buffer = malloc(size);
    return fifo_alloc_common(buffer, size);
}

FifoBuffer *av_fifo_alloc_array(size_t nmemb, size_t size)
{
    void *buffer = calloc(nmemb, size);
    return fifo_alloc_common(buffer, nmemb * size);
}

void av_fifo_free(FifoBuffer *f)
{
    if (f) {
        av_freep(&f->buffer);
        free(f);
    }
}

void av_fifo_freep(FifoBuffer **f)
{
    if (f) {
        av_fifo_free(*f);
        *f = NULL;
    }
}

void av_fifo_reset(FifoBuffer *f)
{
    f->wptr = f->rptr = f->buffer;
    f->wndx = f->rndx = 0;
}

int av_fifo_size(const FifoBuffer *f)
{
    return (uint32_t)(f->wndx - f->rndx);
}

int av_fifo_space(const FifoBuffer *f)
{
    return f->end - f->buffer - av_fifo_size(f);
}

int av_fifo_realloc2(FifoBuffer *f, unsigned int new_size)
{
    unsigned int old_size = f->end - f->buffer;

    if (old_size < new_size) {
        int len          = av_fifo_size(f);
        FifoBuffer *f2 = av_fifo_alloc(new_size);

        if (!f2)
            return -1;
        av_fifo_generic_read(f, f2->buffer, len, NULL);
        f2->wptr += len;
        f2->wndx += len;
        free(f->buffer);
        *f = *f2;
        free(f2);
    }
    return 0;
}

int av_fifo_grow(FifoBuffer *f, unsigned int size)
{
    unsigned int old_size = f->end - f->buffer;
    if(size + (unsigned)fifo_size(f) < size)
        return -1;

    size += fifo_size(f);

    if (old_size < size)
        return fifo_realloc2(f, FFMAX(size, 2*old_size));
    return 0;
}

/* src must NOT be const as it can be a context for func that may need
 * updating (like a pointer or byte counter) */
int av_fifo_generic_write(FifoBuffer *f, void *src, int size,
                          int (*func)(void *, void *, int))
{
    int total = size;
    uint32_t wndx= f->wndx;
    uint8_t *wptr= f->wptr;

    do {
        int len = FFMIN(f->end - wptr, size);
        if (func) {
            len = func(src, wptr, len);
            if (len <= 0)
                break;
        } else {
            memcpy(wptr, src, len);
            src = (uint8_t *)src + len;
        }
// Write memory barrier needed for SMP here in theory
        wptr += len;
        if (wptr >= f->end)
            wptr = f->buffer;
        wndx    += len;
        size    -= len;
    } while (size > 0);
    f->wndx= wndx;
    f->wptr= wptr;
    return total - size;
}

int av_fifo_generic_peek_at(FifoBuffer *f, void *dest, int offset, int buf_size, void (*func)(void*, void*, int))
{
    uint8_t *rptr = f->rptr;

    assert(offset >= 0);

    /*
     * *ndx are indexes modulo 2^32, they are intended to overflow,
     * to handle *ndx greater than 4gb.
     */
    assert(buf_size + (unsigned)offset <= f->wndx - f->rndx);

    if (offset >= f->end - rptr)
        rptr += offset - (f->end - f->buffer);
    else
        rptr += offset;

    while (buf_size > 0) {
        int len;

        if (rptr >= f->end)
            rptr -= f->end - f->buffer;

        len = FFMIN(f->end - rptr, buf_size);
        if (func)
            func(dest, rptr, len);
        else {
            memcpy(dest, rptr, len);
            dest = (uint8_t *)dest + len;
        }

        buf_size -= len;
        rptr     += len;
    }

    return 0;
}

int av_fifo_generic_peek(FifoBuffer *f, void *dest, int buf_size,
                         void (*func)(void *, void *, int))
{
    
    // Read memory barrier needed for SMP here in theory
    uint8_t *rptr = f->rptr;

    do {
        int len = FFMIN(f->end - rptr, buf_size);
        if (func)
            func(dest, rptr, len);
        else {
            memcpy(dest, rptr, len);
            dest = (uint8_t *)dest + len;
        }
        
        // memory barrier needed for SMP here in theory
        rptr += len;
        if (rptr >= f->end)
            rptr -= f->end - f->buffer;
        buf_size -= len;
    } while (buf_size > 0);

    return 0;
}

int av_fifo_generic_read(FifoBuffer *f, void *dest, int buf_size,
                         void (*func)(void *, void *, int))
{
    
    // Read memory barrier needed for SMP here in theory
    do {
        int len = FFMIN(f->end - f->rptr, buf_size);
        if (func)
            func(dest, f->rptr, len);
        else {
            memcpy(dest, f->rptr, len);
            dest = (uint8_t *)dest + len;
        }
        
        // memory barrier needed for SMP here in theory
        av_fifo_drain(f, len);
        buf_size -= len;
    } while (buf_size > 0);
    return 0;
}

/** Discard data from the FIFO. */
void av_fifo_drain(FifoBuffer *f, int size)
{
    assert(av_fifo_size(f) >= size);
    f->rptr += size;
    if (f->rptr >= f->end)
        f->rptr -= f->end - f->buffer;
    f->rndx += size;
}
