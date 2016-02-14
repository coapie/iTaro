
#include "usmm.h"
#include "usmm_slab.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h> 

static void dump_bitmap(void *ptr, int max){
    unsigned char *p = (unsigned char *)ptr;
    int num = max / 8;
    int i;

    for(i = 0; i < num; i++, p++){
        printf("0x%02x,", (*p));

        if((i != 0) && ((i % 16) == 0))
            printf("\n");
    }
    printf("\n");
}


static usmm_t *usmm_test_init(uint64_t addr, int fd, size_t flen, size_t slab){
    usmm_conf_t conf;
    usmm_t *us;

    printf("len %ld slab %ld\n", flen, slab); 
   
    conf.addr = (void *)addr;
    conf.length = flen;
    conf.slab = slab;
    conf.fd = fd;

    if(usmm_init(&conf, &us) != 0){
        printf("usmm init fail!\n");
        return NULL;
    }

    printf("usmm init len %ld slab %ld\n", us->conf.length, us->conf.slab);
    return us;
}

static int usmm_test_heap(usmm_t *us){
    void *h;
    int i;

    printf("usmm-heap test...\n");

    for(i = -1024; i < us->conf.slab*8; i += (int)us->conf.slab){
        h = usmm_heap_alloc(us, us->conf.slab + i);
        if(h == NULL){
            printf("alloc heap %d fail!\n", i);
            return -1;
        }
    }

    printf("usmm-heap test more...\n");

    for(i = 1024; i < us->conf.slab*8; i += (int)us->conf.slab){
        h = usmm_heap_alloc(us, us->conf.slab + i);
        if(h == NULL){
            printf("alloc heap %d fail!\n", i);
            return -1;
        }
//        printf("alloc heap %d \n", us->conf.slab+i);

       usmm_heap_free(us, h);
    }

    printf("usmm heap alloc 100 slab...\n");
    h = usmm_heap_alloc(us, us->conf.slab * 100);
    if(h == NULL){
        printf("alloc heap for 100*slab fail!\n");
        return -1;
    }
    usmm_heap_free(us, h);

    dump_bitmap(us->slab_bitmap, 1024);
    h = usmm_heap_alloc(us, 1024*1024*1024);
    if(h == NULL){
        printf("alloc heap for chunk size fail!\n");
        return -1;
    }
    usmm_heap_free(us, h);

    printf("usmm heap alloc > 1GB ...\n");

    h = usmm_heap_alloc(us, 1024*1024*1024 + 200);
    if(h == NULL){
        printf("alloc heap for  not fail!\n");
        return -1;
    }
    usmm_heap_free(us, h);
    
    printf("usmm heap test ok!\n");
    return 0;
}

static int usmm_test_slab(usmm_t *us){
    void* tbuf[64][64];
    int i,j;
    void *ptr;
    size_t size;

    printf("usmm slab test ...\n");

    for(i = 0; i < 64; i ++){
        size = 4 + i*16;
        for(j = 0; j < 64; j++){
            size += j*64;
            ptr = usmm_slice_alloc(us, size);
            if(ptr == NULL){
                printf("alloc size %ld fail!\n", size);
                return -1;
            }
            memset(ptr, 0, size);
            tbuf[i][j] = ptr;
        }
    }
    
    for(i = 0; i < 64; i++){
        for(j = 0; j < 64; j++){
            usmm_slice_free(us, tbuf[i][j]);
            tbuf[i][j] = NULL;
        }
    }

    for(i = 0; i < 64; i++){
        size = 1024 + 1024*i;
        for(j = 0; j < 64; j++){
            size += j *1024; 
            ptr = usmm_slice_alloc(us, size);
            if((ptr == NULL) && (size < 128*1024)){
                printf("alloc size %ld fail!\n", size);
                return -1;
            }

            if(ptr != NULL)
                memset(ptr, 0, size);
            tbuf[i][j] = ptr;
        }
    }

    for(i = 0; i < 64; i++){
        for(j = 0; j < 64; j++){
            usmm_slice_free(us, tbuf[i][j]);
            tbuf[i][j] = NULL;
        }
    }

    printf("test slab ok\n");
    return 0;
}

static int random_size(int max){
    int r;
    
    r = rand() % max;
    if(r == 0)
        r = 2;

    return r;
}

static void **hash_table = NULL;
static int hash_max = 0;

static int hash_init(usmm_t *us, int max){
    hash_table = (void**)malloc(sizeof(void*) * max);
    if(hash_table == NULL){
        return -1;
    }
    memset(hash_table, 0, sizeof(void*) * max);

    hash_max = max;

    return 0;
}

static int hash_fini(usmm_t *us){
    void *ptr;
    int i;
    int count = 0;

    for(i = 0; i < hash_max; i++){
        ptr = hash_table[i];
        if(ptr != NULL){
            usmm_slice_free(us, ptr);
            hash_table[i] = NULL;
            count ++;
        }
    }

    return count;
}

static int hash_set(usmm_t *us, void *ptr){
    int idx = rand() % hash_max;
    void *ptr_old = hash_table[idx];

    if(ptr_old != NULL){
        usmm_slice_free(us, ptr_old);
        hash_table[idx] = ptr;
        return 1;
    }else{
        hash_table[idx] = ptr;
        return 0;
    }
}

static void usmm_slab_random_test(usmm_t *us){
    int max =  1024;
    int size;
    int i;
    void *ptr;
    
    hash_init(us, max);

    for(i = 0; i < 0x000fffff; i++){
        size = random_size(128*1024);
        ptr = usmm_slice_alloc(us, size);
        if(ptr != NULL){
            memset(ptr, 0, size);
            hash_set(us, ptr);
        }
    }

    hash_fini(us);
}

/*
void *usmm_slot_set(usmm_t *us, int slot, void *ptr);
void *usmm_slot_get(usmm_t *us, int slot);
int usmm_slot_append(usmm_t *us, void *ptr);
*/

#define USMM_TEST_MAP_ADDR      0x00003AAA00000000
#define USMM_TEST_SLAB          USMM_DEFAULT_SLAB
#define USMM_TEST_MAP_LEN       0x0000000080000000    // 2GB

int main(int argc, char *argv[]){
    usmm_t *us;
    int fd;

    if(argc < 2){
        printf("usage usmm tempfile\n");
        return -1;
    }

    fd = open(argv[1], O_RDWR);
    if (fd == -1){
        printf("open temp file %s fail!\n", argv[1]);
        return -1;
    }

    if(ftruncate(fd, USMM_TEST_MAP_LEN) < 0){
        printf("truncate file fail!\n");
        return -1;
    }

    us = usmm_test_init(USMM_TEST_MAP_ADDR, fd, USMM_TEST_MAP_LEN, USMM_TEST_SLAB);
    if(us == NULL){
        printf("init test environment fail!\n");
        return -1;
    }

    usmm_test_heap(us);

    usmm_test_slab(us);

    usmm_slab_random_test(us);

    usmm_stat(us);

    usmm_fini(us);

    return 0;
}


