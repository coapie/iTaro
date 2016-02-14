
#include "usmm_slab.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SLAB_SIZE       (1024*1024*2)
#define SLICE_SIZE      64

static void usmm_slab_dump(usmm_slab_t *slab){
    printf("slab :\n");
    printf("     size:%d\n", slab->size);
    printf("    slice:%d\n", slab->slice);
    printf("     used:%d\n", slab->used);
    printf("    total:%d\n", slab->total);
}

static int usmm_slab_test(){
    usmm_slab_t *slab;
    int i, total;
    void *ptr;

     slab = (usmm_slab_t *)malloc(SLAB_SIZE);
    if(slab == NULL){
        printf("alloc buffer for slab test fail!\n");
        return -1;
    }

    if(usmm_slab_init(slab) != 0){
        printf("init slab fail!\n");
        return -1;
    }

    if(usmm_slab_create(slab, SLAB_SIZE, SLICE_SIZE) != 0){
        printf("create slab fail!\n");
        return -1;
    }

    total = (SLAB_SIZE - sizeof(*slab)) / SLICE_SIZE;

    if(usmm_slab_isempty(slab)){
        printf("yes, have all the slice!\n");
    }else{
        printf("something wrong!\n");
        return -1;
    }

    for(i = 0; i < total; i++){
        ptr = usmm_slab_alloc(slab);
        if(ptr == NULL){
            printf("total slice is not get!\n");
            return -1;
        }
    }

    if(usmm_slab_isfull(slab)){
        printf("yes, all slice is allocated!\n");
    }else{
        printf("beyong total slice!\n");
        usmm_slab_dump(slab);
        return -1;
    }

    usmm_slab_fini(slab);

    if(usmm_slab_create(slab, SLAB_SIZE, SLICE_SIZE) != 0){
        printf("create slab fail!\n");
        return -1;
    }

    total = (SLAB_SIZE - sizeof(*slab)) / SLICE_SIZE;

    for(i = 0; i < total; i++){
        ptr = usmm_slab_alloc(slab);
        if(ptr == NULL){
            printf("total slice is not get!\n");
            return -1;
        }
        usmm_slab_free(slab, ptr);
    }

    for(i = 0; i < total; i++){
        ptr = usmm_slab_alloc(slab);
        if(ptr == NULL){
            printf("total slice is not get!\n");
            return -1;
        }
        usmm_slab_free(slab, ptr);
    }

    if(usmm_slab_isempty(slab)){
        printf("yes, all slice is returned!\n");
    }else{
        printf("fail!\n");
        return -1;
    }

    return 0;
}

int main(){
    return usmm_slab_test();
}


