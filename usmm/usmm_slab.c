
#include "usmm_slab.h"
#include "osapi.h"

#include <string.h>
#include <stdint.h>

struct usmm_slice{
    struct usmm_slice *next;
};

static const uint64_t gs_magic = 0xFFEEDDCCBBAA9988;


int usmm_slab_isinit(usmm_slab_t *slab){
    return (memcmp(&gs_magic, slab->magic, 8) == 0);
}


int usmm_slab_init(usmm_slab_t *slab){
    ASSERT(slab != NULL);

    memcpy(slab->magic, &gs_magic, 8);
    INIT_LIST_HEAD(&slab->node);
    slab->size = 0;
    slab->slice = 0;
    slab->total = 0;
    slab->used = 0;
    slab->free_slice = NULL;

    return 0;
}

int usmm_slab_fini(usmm_slab_t *slab){
    ASSERT(slab != NULL);

    memset(slab, 0, sizeof(*slab));

    return 0;
}

int usmm_slab_create(usmm_slab_t *slab, size_t size, size_t slice){
    struct usmm_slice *s;
    void *ptr;
    int i;

    ASSERT(slab != NULL);

//    printf("slab_create size %d slice %d\n", size, slice);

    slab->free_slice = NULL;
    slab->size = size;
    slab->slice = slice;
    slab->total = (size - sizeof(*slab))/ slice;
    ptr = (void *)(slab + 1);

    for(i = 0; i < slab->total; i++){
        s = (void *)(ptr + i * slab->slice);
        s->next = slab->free_slice;
        slab->free_slice = s;
    }

    return 0;
}

void *usmm_slab_alloc(usmm_slab_t *slab){
    struct usmm_slice *slice;

    ASSERT(slab != NULL);

    if(slab->free_slice != NULL){
        slice = (struct usmm_slice *)slab->free_slice;
        slab->free_slice = slice->next;
        slab->used ++;

        return slice;
    }

    return NULL;
}

int usmm_slab_free(usmm_slab_t *slab, void *ptr){
    struct usmm_slice *slice;

    ASSERT(slab != NULL);

    if(ptr == NULL)
        return 0;

    if(slab->used == 0){
        ASSERT(0);
        return -1;
    }

    slice = (struct usmm_slice *)ptr;
    slice->next = slab->free_slice;
    slab->free_slice = slice;
    
    slab->used --;

    return 0;
}


