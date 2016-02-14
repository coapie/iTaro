
#ifndef __USMM_SLAB_H__
#define __USMM_SLAB_H__

#include "list.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef struct usmm_slab{
    char magic[8];
    struct list_head node;    
    size_t size;    // total size of this slab
    size_t slice;   // slice size of this slab
    
    int total;
    int used;
    void *free_slice;
}usmm_slab_t;

int usmm_slab_init(usmm_slab_t *slab);
int usmm_slab_fini(usmm_slab_t *slab);

int usmm_slab_create(usmm_slab_t *slab, size_t size, size_t slice);

void *usmm_slab_alloc(usmm_slab_t *slab);
int usmm_slab_free(usmm_slab_t *slab, void *ptr);

int usmm_slab_isinit(usmm_slab_t *slab);


static inline int usmm_slab_isempty(usmm_slab_t *slab){
    return (slab->used == 0);
}

static inline int usmm_slab_isfull(usmm_slab_t *slab){
    return ((slab->used == slab->total) && (slab->used != 0));
}

#ifdef __cplusplus
}
#endif

#endif //__USMM_SLAB_H__


