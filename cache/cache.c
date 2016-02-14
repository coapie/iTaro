#include "cache.h"
#include "dict.h"
#include "zmalloc.h"
#include "cobj.h"

#include "logger.h"

#include <string.h>
#include <errno.h>

static int dictSdsKeyCompare(void *privdata, const void *key1,
        const void *key2)
{
    int l1,l2;
    DICT_NOTUSED(privdata);

    l1 = sdslen((sds)key1);
    l2 = sdslen((sds)key2);
    if (l1 != l2) return 0;
    return memcmp(key1, key2, l1) == 0;
}

static void dictCObjDestructor(void *privdata, void *val)
{
    DICT_NOTUSED(privdata);

    if (val == NULL) return; /* Values of swapped out keys as set to NULL */

    // FIXME :
//    decrRefCount(val);
}

static void dictSdsDestructor(void *privdata, void *val)
{
    DICT_NOTUSED(privdata);

    sdsfree(val);
}


static unsigned int dictSdsHash(const void *key) {
    return dictGenHashFunction((unsigned char*)key, sdslen((char*)key));
}

static dictType cacheDictType = {
    dictSdsHash,            /* hash function */
    NULL,                       /* key dup */
    NULL,                       /* val dup */
    dictSdsKeyCompare,          /* key compare */
    dictSdsDestructor,         /* key destructor */
    dictCObjDestructor          /* val destructor */
};

cache_t *cache_create(){
    cache_t *ce = zmalloc(sizeof(*ce));
    if(ce == NULL){
        log_warn("no memory for create cache instance!\n");
        return NULL;
    }

    ce->dict = dictCreate(&cacheDictType, NULL);
    if(ce->dict == NULL){
        log_warn("create dict for cache instance fail!\n");
        zfree(ce);
        return NULL;
    }

    return ce;
}

int cache_destroy(cache_t *ce){
    //FIXME : 
    return -1;
}

int cache_set(cache_t *ce, sds_t key, cobj_t *val, int expire){

    return dictAdd(ce->dict, key, val);
}

int cache_get(cache_t *ce, sds_t key, cobj_t **val){
    dictEntry *de;

    de = dictFind(ce->dict, key);
    if(de != NULL){
        *val = dictGetVal(de);
        return 0;
    }

    return -ENOENT;
}

int cache_del(cache_t *ce, sds_t key){
    return dictDelete(ce->dict, key);
}

int cache_expire(cache_t *ce, sds_t key, int expire){
    // FIXME :
    return -1;
}


