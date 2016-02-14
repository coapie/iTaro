
#include "cobj.h"
#include "sds.h"
#include "logger.h"
#include "zmalloc.h"
#include "osapi.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// key-value cache interface
cobj_t *cobj_sds_create(sds_t val){
    cobj_t *co;

    co = zmalloc(sizeof(*co));
    if(co == NULL){
        log_warn("no memory for cobj alloc!\n");
        return NULL;
    }
    
    co->type = COBJ_STRING;
    co->encoding = COBJ_ENCODING_RAW;
    co->lru = 0;
    co->refcount = 1;
    co->ptr = val;

    return co;
}

int cobj_sds_destroy(cobj_t *co){
    // FIXME :
    return -1;
}

static int sds_to_int64(sds_t val, int64_t *out){
    char *endptr;
    int64_t num;

    ASSERT((val != NULL) && (out != NULL));

    errno = 0;    /* To distinguish success/failure after call */
    num = strtoll(val, &endptr, 10);
    if (errno != 0) {
        log_warn("conver sds to int64 fail!\n");
        return -errno;
    }

    if (endptr == val) {
        log_warn("sds have no number!\n");
        return -EINVAL;
    }

    if (*endptr != '\0'){
        log_warn("sds have charactor no in number!\n");
        return -EINVAL;
    }

    *out = num;
    return 0;
}

static inline int cobj_incrby(cobj_t *co, int incr){
    int64_t num = 0;
    sds_t newsds;

    if(co->type != COBJ_STRING){
        log_warn("only string can be incr!\n");
        return -1;
    }

    if(co->encoding == COBJ_ENCODING_INT){
        num = (int64_t)co->ptr;
    }else{
        if(sds_to_int64((sds_t)co->ptr, &num) != 0){
            log_warn("convert sds to number fail!\n");
            return -EINVAL;
        }
    }

    if ((incr < 0 && num < 0 && incr < (LLONG_MIN-num)) ||
        (incr > 0 && num > 0 && incr > (LLONG_MAX-num))) {
        log_warn("increment or decrement would overflow\n");
        return -EINVAL;
    }

    num += incr;

    if(co->encoding == COBJ_ENCODING_INT){
        co->ptr = (void *)num;
    }else{
        newsds = sdsfromlonglong(num);
        sdsfree((sds_t)co->ptr);
        co->ptr = newsds;
    }

    return 0;
}

int cobj_sds_incrby(cobj_t *co, int incr){
    return cobj_incrby(co, incr);
}
    

int cobj_sds_decrby(cobj_t *co, int decr){
    return cobj_incrby(co, -decr);
}

// hash table cache interface
cobj_t *cobj_htable_create(){
    return NULL;
}

int cobj_htable_destroy(cobj_t *co){
    return -1;
}

int cobj_htable_set(cobj_t *co, sds_t field, sds_t value){
    return -1;
}

int cobj_htable_get(cobj_t *co, sds_t field, sds_t *value){
    return -1;
}

int cobj_htable_del(cobj_t *co, sds_t field){
    return -1;
}

int cobj_htable_incrby(cobj_t *co, sds_t field, int incr){
    return -1;
}

int cobj_htable_decrby(cobj_t *co, sds_t field, int decr){
    return -1;
}

int cobj_htable_exists(cobj_t *co, sds_t field){
    return -1;
}

int cobj_htalbe_length(cobj_t *co){
    return -1;
}

