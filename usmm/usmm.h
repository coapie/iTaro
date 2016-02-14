
#ifndef __USMM_H__
#define __USMM_H__

#include "list.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif

#define USMM_DEFAULT_CHUNK      (1024*1024*1024)    // 1GB
#define USMM_DEFAULT_SLAB       (2*1024*1024)       // 2MB as huge page size

#define USMM_ALIGN(n,s)     ((n + s - 1) & (~(s -1)))

enum {
    USMM_SLOT_RESERVED = 0,
    USMM_SLOT_CACHE,
    USMM_SLOT_APP = 20,
    // add your app ...
};

typedef struct usmm_slot{
    int slot_id;
    void *ptr;
}usmm_slot_t;

typedef struct usmm_conf{
    void *addr;         // for mmap addr
    size_t length;      // max length of this usmm can used.

    size_t slab;        // slab size

    int fd;             // mmap file handle
}usmm_conf_t;

typedef struct usmm_slice_stat{
    uint64_t ops;
    uint64_t used;

    uint32_t slabs;
    uint32_t full;
}usmm_slice_stat_t;

typedef struct usmm{
    char magic[32];
    usmm_conf_t  conf;

    char *slab_bitmap;
    usmm_slot_t *slots;
    
    int slab_num;
    struct list_head *slabs;

    usmm_slice_stat_t *stat_slices;

    uint32_t stat_free_slabs;
    uint32_t stat_full_slabs;

    struct list_head free_slabs;
    struct list_head full_slabs;
}usmm_t;

int usmm_init(usmm_conf_t *conf,  usmm_t **us);
int usmm_fini(usmm_t *us);

void *usmm_heap_alloc(usmm_t *us, size_t size);
void usmm_heap_free(usmm_t *us, void *ptr);

void *usmm_slice_alloc(usmm_t *us, size_t size);
void  usmm_slice_free(usmm_t *us, void *ptr);

void *usmm_slot_set(usmm_t *us, int slot, void *ptr);
void *usmm_slot_get(usmm_t *us, int slot);
int usmm_slot_append(usmm_t *us, void *ptr);

void usmm_stat(usmm_t *us);

#ifdef __cplusplus
}
#endif

#endif //__USMM_H__

