
#include "dict.h"
#include "sds.h"
#include "cobj.h"

#ifndef __CACHE_H__
#define __CACHE_H__


typedef struct cache{
    dict *dict;
}cache_t;


// cache base interface
cache_t *cache_create();
int cache_destroy();

int cache_set(cache_t *ce, sds_t key, cobj_t *val, int expire);
int cache_get(cache_t *ce, sds_t key, cobj_t **val);
int cache_del(cache_t *ce, sds_t key);
int cache_expire(cache_t *ce, sds_t key, int expire);

// key-value cache interface
cobj_t *cobj_sds_create(sds_t val);
int cobj_sds_destroy(cobj_t *co);
int cobj_sds_incrby(cobj_t *co, int incr);
int cobj_sds_decrby(cobj_t *co, int decr);

// hash table cache interface
cobj_t *cobj_htable_create();
int cobj_htable_destroy(cobj_t *co);
int cobj_htable_set(cobj_t *co, sds_t field, sds_t value);
int cobj_htable_get(cobj_t *co, sds_t field, sds_t *value);
int cobj_htable_del(cobj_t *co, sds_t field);
int cobj_htable_incrby(cobj_t *co, sds_t field, int incr);
int cobj_htable_decrby(cobj_t *co, sds_t field, int decr);
int cobj_htable_exists(cobj_t *co, sds_t field);
int cobj_htalbe_length(cobj_t *co);

#endif //__CACHE_H__

