
#include "bitmap.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void test_dump(void *ptr, int max){
    unsigned int *p = (unsigned int *)ptr;
    int num = max / 32;
    int i;

    for(i = 0; i < num; i++, p++){
        printf("0x%08x,", (*p));

        if((i != 0) && ((i % 8) == 0))
            printf("\n");
    }
    printf("\n");
}

static void *init(int bitsize){
    int size = bitsize / 8;

    void *ptr = malloc(size);
    if(ptr == NULL)
        return NULL;

    memset(ptr, 0, size);

    return ptr;
}

static void fini(void *ptr){
    free(ptr);
}

static void bitset_test(void *ptr, int idx){
    bitmap_set(ptr, idx);
    if(!bitmap_test(ptr, idx))
        printf("set %d fail\n", idx);
    bitmap_clr(ptr, idx);
}

static void test_set(void *ptr, int max){
    bitset_test(ptr, 0);
    bitset_test(ptr, 7);
    bitset_test(ptr, 8);
    bitset_test(ptr, 15);
    bitset_test(ptr, 16);
    bitset_test(ptr, 31);
    bitset_test(ptr, 32);
    bitset_test(ptr, 63);
    bitset_test(ptr, 64);
    bitset_test(ptr, 255);
    bitset_test(ptr, 221);
    bitset_test(ptr, 133);

//    test_dump(ptr, max);
}

static void test_find_range(void *ptr, int max){
    // case 0 : range in uint64_t;
    int idx;

    bitmap_set_range(ptr, 0, max);
    bitmap_clr_range(ptr, 2, 30);
    // test_dump(ptr, max);
    idx = bitmap_find_range(ptr, 16, max);
    if(idx != 8){
        printf("case 0 fail %d\n", idx);
        return ;
    }

    //printf("===========\n\n");
    // case 1 : range in tailer of uint64_t;
    bitmap_set_range(ptr, 0, max);
    //test_dump(ptr, max);
    //printf("----------------\n");
    bitmap_clr_range(ptr, 120, 159);
    //test_dump(ptr,max);
    //printf("============\n\n");
    idx = bitmap_find_range(ptr, 5, max);
    if(idx != 120){
        printf("case 1 fail -1 %d!\n", idx);
        return ;
    }

    bitmap_clr_range(ptr, 120, 256);
    idx = bitmap_find_range(ptr, 126, max);
    if(idx != 120){
        printf("case 1 fail -2 %d!\n", idx);
        return ;
    }

    // case 2 : range in headerer of uint64_t;
    bitmap_set_range(ptr, 0, max);
    bitmap_clr_range(ptr, 120, 320);
    idx = bitmap_find_range(ptr, 150, max);
    if(idx != 120){
        printf("case 2 fail -1 %d!\n", idx);
        return ;
    }

    printf("bitmap ok!\n");
}

int main(){
    void *ptr = init(4096);
    if(ptr == NULL){
        printf("no memory!\n");
        return -1;
    }

    test_set(ptr, 4096);

    test_find_range(ptr, 4096);
    
    return 0;
}


