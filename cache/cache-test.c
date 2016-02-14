
#include "cache.h"
#include "cobj.h"
#include "sds.h"
#include "dict.h"

#include "logger.h"

#include <string.h>

const char *kHello = "Hello ";
const char *kWorld = "World!";

int cache_test(){

    sds_t key0;
    sds_t key1;

    cobj_t val0;
    cobj_t *val1;

    int ret;

    cache_t *ce = cache_create();
    if(ce == NULL){
        log_error("create cache instance fail!\n");
        return -1;
    }

    key0 = sdsnew("key for hello");

    cobj_init(&val0, COBJ_STRING, COBJ_ENCODING_RAW, kHello);

    ret = cache_set(ce, key0, &val0, 0);
    if(ret != 0){
        log_error("cache set key fail!\n");
        return -1;
    }

    ret = cache_get(ce, key0, &val1);
    if(ret != 0){
        log_error("cache get key fail!\n");
        return -1;
    }

    log_info("cache test ok!\n");

    return 0;
}

int main(){
    
    cache_test();

    return 0;
}



