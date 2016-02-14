
#include "bitmap.h"
#include "osapi.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>


void bitmap_set(void *bitmap, int index){
    uint8_t *ptr = (uint8_t*)bitmap;
    int idx = index >> 3;       // index / 8;
    uint8_t bits = index & 0x07;    // index % 8
    
    ptr += idx;
    *ptr = (*ptr) | (1 << bits);
}

void bitmap_clr(void *bitmap, int index){
    uint8_t *ptr = (uint8_t*)bitmap;
    int idx = index >> 3;       // index / 8
    uint8_t bits = index & 0x07;    //index % 8;

    ptr += idx;
    *ptr =(*ptr) & ( ~(1 << bits));
}

void bitmap_set_range(void *bitmap, int start, int end){
    // slow but works
    int i;

    for (i = start; i < end; i++){
        bitmap_set(bitmap, i);
    }
}

void bitmap_clr_range(void *bitmap, int start, int end){
    int i;

    for(i = start; i < end; i++){
        bitmap_clr(bitmap, i);
    }
}

int bitmap_find_range(void *bitmap, int range, int max){
    uint8_t max_dw = 0xff;
    int rin = range % 8;
    int count = range / 8;
    uint8_t lowrin  = ~(max_dw << rin);
    uint8_t highrin = ~(max_dw >> rin);
    register uint8_t *ptr;
    register int i;
    register int j;

    ASSERT(range > 1);

//    printf("find range %d %d\n", range, max);

    if(count == 0){
        ptr = (uint8_t*)bitmap;
        for(i = 0; i < max / 8; i++, ptr++){
            if(*ptr != max_dw){
                for(j = 0; j < (8 - rin); j++){
                    if(!((*ptr) & (lowrin << j))){
                        return i*8 + j;
                    }
                }
            }
        }

        return -1;
    }

    ptr = (uint8_t *)bitmap;
    for(i = 0; i < (max - count - 1) / 8; i++, ptr++){
        if(! ((*ptr) & lowrin)){
            if((*ptr) != 0){
                for(j = 0; j < count; j++){
                    if((*ptr) != 0)
                        break;
                }

                if(j == count){
                    return i*8 + 8 - rin;
                }
            }else{
                for(j = 1; j < count; j++){
                    if((*(ptr + j)) != 0)
                        break;
                }

                if(j == count){
                    if(! ((*(ptr + j)) & highrin)){
                        return i*8;
                    }
                }
            }
        }
    }

    return -1;
}

int bitmap_test(void *bitmap, int index){
    uint8_t *ptr = (uint8_t*)bitmap;
    int idx = index >> 3;       // index / 8;
    uint8_t bits = index & 0x07;    //index % 8;

    ptr += idx;
    return  ((*ptr) & (1 << bits)) != 0;
}


