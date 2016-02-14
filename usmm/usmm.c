
#include "usmm.h"
#include "usmm_slab.h"
#include "bitmap.h"

#include "logger.h"
#include "list.h"
#include "osapi.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>

/*
 * User-mode Shared Memory Management:

    chunk is not used, ignore it!!

 * [--usmm--]  => usmm { chunk+ }
 *  ->[--1GB--][--1GB--][--1GB--]...[--1GB--]  => chunk { slab+ }
 *    ->[--2MB--][--2MB--][--2MB--]...[--2MB--]  => slab { slice+ }
 * slab => [slab-header, [slice+]]
 * ----------------------------------------------------------------
 * USMM controller in first slab
 * { usmm-cb[32kB], stat-slice[32kB], slab-link-header[32kB], 
 *   slots[32kB *16], reserver[32kB * 13], slab-bitmap[1024kB] }
 * max-slab is : 1024k * 8 = 8M
 * max-slot is : 32k
 */
#define USMM_STAT_SLICE_OFFSET          (32*1024)
#define USMM_SLABS_OFFSET               (64*1024)   // 64kB
#define USMM_SLAB_BITMAP_OFFSET         (1024*1024) // 1024kB
#define USMM_SLOTS_OFFSET               (96*1024)

#define USMM_SLAB_BITMAP_SIZE           (1024*1024*8)
#define USMM_SLOTS_MAX                  (32*1024)

static const char gs_magic[32] = "usmm-0xabcde9876543210-0xabefc.\0";

#define USMM_SLAB_ARRAY_MAX     41
#define kb                      1024

static const int gs_slab_array[] = {
    8,
    16,   32,   48,   64,   80,   96,   112,   128,     // step in 16
    192,  256,  320,  384,  448,  512,                  // step in 64
    768,  1024, 1280, 1536, 1792, 2048,                 // step in 256
    4*kb, 6*kb, 8*kb, 10*kb, 12*kb, 14*kb, 16*kb,       // step in 2kb
    20*kb,24*kb, 28*kb, 32*kb, 36*kb, 40*kb, 44*kb, 48*kb, // step in 4kb
    64*kb, 80*kb, 96*kb, 112*kb, 128*kb                 // step in 16kb
};

static int usmm_slab_find(usmm_t *us, int size, int *slice){
    int idx = 0;

    ASSERT((us != NULL) && (slice != NULL));    

    if(size <= 8){
        // [8]
        idx = 0;
    }else if(size <= 128){
        // [16,32,48,64,80,96,112,128]
        size = USMM_ALIGN(size, 16);
        if(size < 16)
            size = 16;
        idx = ((size - 16) / 16) + 1; // [8] 
    }else if(size <= 512){
        // [192,256,320,384,448,512]
        size = USMM_ALIGN(size, 64);
        if(size < 192)
            size = 192;
        idx = ((size - 192) / 64) + 9;
    }else if(size <= 2048){
        // [768, 1024, 1280, 1536, ... 2048]
        size = USMM_ALIGN(size, 256);
        if(size < 768)
            size = 768;
        idx = ((size - 768) / 256) + 15;
    }else if(size <= (16*kb)){
        // [4k, 6k, 8k, ...16k]
        size = USMM_ALIGN(size, 2*kb);
        if(size < 4*kb)
            size = 4*kb;
        idx = ((size - 4*kb) / (2*kb)) + 21;
    }else if(size <= (48*1024)){
        // [20k, 24k, 28k, ....48k]
        size = USMM_ALIGN(size, 4*kb);
        if(size < 20*kb)
            size = 20*kb;
        idx = ((size - 20*kb) / (4*kb)) + 28; 
    }else if(size <= (128*kb)){
        size = USMM_ALIGN(size, 16*kb);
        if(size < (64*kb))
            size = 64*kb;
        idx = ((size - 64*kb) / (16*kb)) + 36;
    }else{
        return -1;
    }

    if(slice != NULL)
        *slice = gs_slab_array[idx];

    return idx;
}

int usmm_init(usmm_conf_t *conf,  usmm_t **us){
    void *map_addr;
    usmm_t *u;
    usmm_slab_t *slab;
    uint64_t offset;
    int i;
    struct list_head *lh;

    ASSERT((conf != NULL) && (us != NULL));

    // map first chunk
    map_addr = mmap(conf->addr, conf->length, PROT_WRITE|PROT_READ, 
        MAP_FIXED|MAP_SHARED|MAP_NORESERVE, conf->fd, 0);
    if(map_addr == MAP_FAILED){
        log_warn("mmap file fail:%d\n", errno);
        return -1;
    }

    u = (usmm_t *)map_addr;

    if(memcmp(gs_magic, u->magic, 32) == 0){
        // case 0 : load usmm
        // reopen shared memory, load ity
        log_info("=====reload usmm controller data=====\n");
        u->stat_slices = (usmm_slice_stat_t*)(map_addr + USMM_STAT_SLICE_OFFSET);
        u->slabs = (struct list_head*)(map_addr + USMM_SLABS_OFFSET);
        u->slab_bitmap  = (void *)(map_addr + USMM_SLAB_BITMAP_OFFSET);
        u->slots = (usmm_slot_t*)(map_addr + USMM_SLOTS_OFFSET);  
    }else{
        // case 1 : init usmm
        log_info("=====init usmm controller data=====\n");
        memset(map_addr, 0, conf->slab);
        memcpy(u->magic, gs_magic, 32);
        memcpy(&u->conf, conf, sizeof(*conf));

        u->stat_slices = (usmm_slice_stat_t*)(map_addr + USMM_STAT_SLICE_OFFSET);
        u->slabs = (struct list_head *)(map_addr + USMM_SLABS_OFFSET);
        u->slab_bitmap  = (void *)(map_addr + USMM_SLAB_BITMAP_OFFSET);
        u->slots = (usmm_slot_t*)(map_addr + USMM_SLOTS_OFFSET);

        bitmap_set_range(u->slab_bitmap, 0, USMM_SLAB_BITMAP_SIZE);
        bitmap_clr_range(u->slab_bitmap, 0, u->conf.length / u->conf.slab);
        bitmap_set(u->slab_bitmap, 0);

        INIT_LIST_HEAD(&u->full_slabs);
        INIT_LIST_HEAD(&u->free_slabs);
        
        // skip 0 for header!
        for(i = 1; i < conf->length / conf->slab; i++){
            offset = i*conf->slab;
            slab = (usmm_slab_t *)(map_addr + offset);
            usmm_slab_init(slab);
            list_add_tail(&slab->node, &u->free_slabs);

            u->stat_free_slabs ++;
        }

        for(i = 0; i < USMM_SLAB_ARRAY_MAX; i++){
            lh = u->slabs + i;
            INIT_LIST_HEAD(lh); 
        }
    }


    *us = u;

    return 0;
}

int usmm_fini(usmm_t *us){
    return -1;
}

void *usmm_heap_alloc(usmm_t *us, size_t size){
    usmm_slab_t *slab;
    int i, num, start;
    uint32_t *nlen;

    ASSERT(us != NULL);

    size = USMM_ALIGN(size, us->conf.slab);
    num = size / us->conf.slab;

    start = bitmap_find_range(us->slab_bitmap, num, us->conf.length / us->conf.slab);
    if(start < 0) {
        log_warn("alloc memory size is too bigger!\n");
        return NULL;
    }
    
    bitmap_set_range(us->slab_bitmap, start, start + num);

    for(i = start; i < (start + num); i++){
        slab = (usmm_slab_t *)((void *)us + i*us->conf.slab);
        list_del(&slab->node);
        usmm_slab_fini(slab);
    }

    us->stat_free_slabs -= num;

    nlen = (uint32_t *)((void*)us + start * us->conf.slab);
    *nlen = num; 

    nlen += 1;
    return (void*)nlen;
}

void usmm_heap_free(usmm_t *us, void *ptr){
    usmm_slab_t *slab;
    uint32_t *nlen;
    int index;
    int i, num;

    ASSERT((us != NULL) && (ptr != NULL));

    nlen = (uint32_t*)(ptr - sizeof(uint32_t));
    ptr = (void*)nlen;
    slab = (usmm_slab_t *)ptr;
    index = (size_t)(ptr - (void*)us) / us->conf.slab;

    num = (int)*nlen;

    for(i = index; i < (index + num); i++){
        usmm_slab_init(slab);
        list_add(&slab->node, &us->free_slabs);
        bitmap_clr(us->slab_bitmap, i);

        ptr += us->conf.slab;
        slab = (usmm_slab_t *)ptr;
    }

    us->stat_free_slabs += num;
}

void *usmm_slice_alloc(usmm_t *us, size_t size){
    usmm_slab_t *slab;
    usmm_slice_stat_t *stat;
    struct list_head *n;
    void *ptr;
    int slice;
    int idx;

    ASSERT(us != NULL);
    
    idx = usmm_slab_find(us, size, &slice);
    if(idx  < 0){
        return NULL;
    }

    
    n = us->slabs + idx;
    stat = us->stat_slices + idx;

    if(list_empty(n)){
        if(list_empty(&us->free_slabs)){
            log_warn("no more memory!\n");
            return NULL;
        }
        slab = list_first_entry(&us->free_slabs, usmm_slab_t, node);
        list_del(&slab->node);

        usmm_slab_create(slab, us->conf.slab, slice);
        list_add(&slab->node, n);

        us->stat_free_slabs --;
        stat->slabs ++;
    }else{
        slab = list_first_entry(n, usmm_slab_t, node);
    }
    
    ptr = usmm_slab_alloc(slab);
    
    stat->ops ++;
    stat->used ++;
    
    if(usmm_slab_isfull(slab)){
        list_del(&slab->node);
        list_add(&slab->node, &us->full_slabs);
        
        stat->full ++;
        us->stat_full_slabs ++;
    }

    return ptr;
}

void usmm_slice_free(usmm_t *us, void *ptr){
    usmm_slab_t *slab;
    usmm_slice_stat_t *stat;
    int full;
    int slice;
    int idx;

    ASSERT(us != NULL);

    if(ptr == NULL)
        return ;

    slab = (usmm_slab_t *)(USMM_ALIGN(((uint64_t)ptr), us->conf.slab)); // up align
    slab = (usmm_slab_t *)((void *)slab - us->conf.slab);
    idx = usmm_slab_find(us, slab->slice, &slice);

//    if(slab->slice != slice){
//        log_info("slab->slice:%d  slice:%d\n", slab->slice, slice);
//    }

    ASSERT(slab->slice == slice);

    stat = us->stat_slices + idx;

    full = usmm_slab_isfull(slab);
    usmm_slab_free(slab, ptr);
    stat->ops ++;
    stat->used --;

    if(full){
        list_del(&slab->node);
        idx = usmm_slab_find(us, slab->slice, &full);
        list_add_tail(&slab->node, (us->slabs + idx));

        stat->full --;
        us->stat_full_slabs --;
    }else if(usmm_slab_isempty(slab)){
        list_del(&slab->node);
        usmm_slab_init(slab);
        list_add(&slab->node, &us->free_slabs);
        
        stat->slabs --;
        us->stat_free_slabs ++;
    }
}


void* usmm_slot_set(usmm_t *us, int slot, void *ptr){
    usmm_slot_t *s;
    void *tmp;

    ASSERT((us != NULL) && (slot < USMM_SLOTS_MAX));

    s = us->slots + slot;
    tmp = s->ptr;
    s->ptr = ptr;

    return tmp;
}

void *usmm_slot_get(usmm_t *us, int slot){
    usmm_slot_t *s;

    ASSERT((us != NULL) && (slot < USMM_SLOTS_MAX));
    
    s = us->slots + slot;

    return s->ptr;
}

int usmm_slot_append(usmm_t *us, void *ptr){
    usmm_slot_t *s;
    int i;

    for(i = 0; i < USMM_SLOTS_MAX; i++){
        s = us->slots + i;
        if(s->ptr == NULL){
            s->ptr = ptr;
            return i;
        }
    }

    return -1;
}

void usmm_stat(usmm_t *us){
    usmm_slice_stat_t *stat = (usmm_slice_stat_t *)us->stat_slices;
    int i;

    printf("free slabs : %d \n", us->stat_free_slabs);
    printf("full slabs : %d \n", us->stat_full_slabs);

    for(i = 0; i < sizeof(gs_slab_array) / sizeof(int); i++){
        printf("slice size : 0x%x \n", gs_slab_array[i]);
        printf("      ops %ld, used %ld, slabs %d, full %d\n", 
                stat->ops, stat->used, stat->slabs, stat->full);
        stat ++; 
    }
}

