/*
 * Copyright (c) 2009-2012, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "sds.h"

#ifndef __COBJ_H__
#define __COBJ_H__

/* Error codes */
#define COBJ_OK                0
#define COBJ_ERR               -1


/* Object types */
#define COBJ_STRING 0
#define COBJ_LIST 1
#define COBJ_SET 2
#define COBJ_ZSET 3
#define COBJ_HASH 4

/* Objects encoding. Some kind of objects like Strings and Hashes can be
 * internally represented in multiple ways. The 'encoding' field of the object
 * is set to one of this fields for this object. */
#define COBJ_ENCODING_RAW 0     /* Raw representation */
#define COBJ_ENCODING_INT 1     /* Encoded as integer */
#define COBJ_ENCODING_HT 2      /* Encoded as hash table */
#define COBJ_ENCODING_ZIPMAP 3  /* Encoded as zipmap */
#define COBJ_ENCODING_LINKEDLIST 4 /* Encoded as regular linked list */
#define COBJ_ENCODING_ZIPLIST 5 /* Encoded as ziplist */
#define COBJ_ENCODING_INTSET 6  /* Encoded as intset */
#define COBJ_ENCODING_SKIPLIST 7  /* Encoded as skiplist */

/* Defines related to the dump file format. To store 32 bits lengths for short
 * keys requires a lot of space, so we check the most significant 2 bits of
 * the first byte to interpreter the length:
 *
 * 00|000000 => if the two MSB are 00 the len is the 6 bits of this byte
 * 01|000000 00000000 =>  01, the len is 14 byes, 6 bits + 8 bits of next byte
 * 10|000000 [32 bit integer] => if it's 10, a full 32 bit len will follow
 * 11|000000 this means: specially encoded object will follow. The six bits
 *           number specify the kind of object that follows.
 *           See the COBJ_RDB_ENC_* defines.
 *
 * Lengths up to 63 are stored using a single byte, most DB keys, and may
 * values, will fit inside. */
#define COBJ_RDB_6BITLEN 0
#define COBJ_RDB_14BITLEN 1
#define COBJ_RDB_32BITLEN 2
#define COBJ_RDB_ENCVAL 3
#define COBJ_RDB_LENERR UINT_MAX

/* When a length of a string object stored on disk has the first two bits
 * set, the remaining two bits specify a special encoding for the object
 * accordingly to the following defines: */
#define COBJ_RDB_ENC_INT8 0        /* 8 bit signed integer */
#define COBJ_RDB_ENC_INT16 1       /* 16 bit signed integer */
#define COBJ_RDB_ENC_INT32 2       /* 32 bit signed integer */
#define COBJ_RDB_ENC_LZF 3         /* string compressed with FASTLZ */

/* AOF states */
#define COBJ_AOF_OFF 0             /* AOF is off */
#define COBJ_AOF_ON 1              /* AOF is on */
#define COBJ_AOF_WAIT_REWRITE 2    /* AOF waits rewrite to start appending */

/* Client flags */
#define COBJ_SLAVE (1<<0)   /* This client is a slave server */
#define COBJ_MASTER (1<<1)  /* This client is a master server */
#define COBJ_MONITOR (1<<2) /* This client is a slave monitor, see MONITOR */
#define COBJ_MULTI (1<<3)   /* This client is in a MULTI context */
#define COBJ_BLOCKED (1<<4) /* The client is waiting in a blocking operation */
#define COBJ_DIRTY_CAS (1<<5) /* Watched keys modified. EXEC will fail. */
#define COBJ_CLOSE_AFTER_REPLY (1<<6) /* Close after writing entire reply. */
#define COBJ_UNBLOCKED (1<<7) /* This client was unblocked and is stored in
                                  server.unblocked_clients */
#define COBJ_LUA_CLIENT (1<<8) /* This is a non connected client used by Lua */
#define COBJ_ASKING (1<<9)     /* Client issued the ASKING command */
#define COBJ_CLOSE_ASAP (1<<10)/* Close this client ASAP */
#define COBJ_UNIX_SOCKET (1<<11) /* Client connected via Unix domain socket */
#define COBJ_DIRTY_EXEC (1<<12)  /* EXEC will fail for errors while queueing */
#define COBJ_MASTER_FORCE_REPLY (1<<13)  /* Queue replies even if is master */
#define COBJ_FORCE_AOF (1<<14)   /* Force AOF propagation of current cmd. */
#define COBJ_FORCE_REPL (1<<15)  /* Force replication of current cmd. */
#define COBJ_PRE_PSYNC (1<<16)   /* Instance don't understand PSYNC. */
#define COBJ_READONLY (1<<17)    /* Cluster client is in read-only state. */
#define COBJ_PUBSUB (1<<18)      /* Client is in Pub/Sub mode. */

/* Client request types */
#define COBJ_REQ_INLINE 1
#define COBJ_REQ_MULTIBULK 2

/* Client classes for client limits, currently used only for
 * the max-client-output-buffer limit implementation. */
#define COBJ_CLIENT_TYPE_NORMAL 0 /* Normal req-reply clients + MONITORs */
#define COBJ_CLIENT_TYPE_SLAVE 1  /* Slaves. */
#define COBJ_CLIENT_TYPE_PUBSUB 2 /* Clients subscribed to PubSub channels. */
#define COBJ_CLIENT_TYPE_COUNT 3

/* Slave replication state. Used in server.repl_state for slaves to remember
 * what to do next. */
#define COBJ_REPL_NONE 0 /* No active replication */
#define COBJ_REPL_CONNECT 1 /* Must connect to master */
#define COBJ_REPL_CONNECTING 2 /* Connecting to master */
/* --- Handshake states, must be ordered --- */
#define COBJ_REPL_RECEIVE_PONG 3 /* Wait for PING reply */
#define COBJ_REPL_SEND_AUTH 4 /* Send AUTH to master */
#define COBJ_REPL_RECEIVE_AUTH 5 /* Wait for AUTH reply */
#define COBJ_REPL_SEND_PORT 6 /* Send REPLCONF listening-port */
#define COBJ_REPL_RECEIVE_PORT 7 /* Wait for REPLCONF reply */
#define COBJ_REPL_SEND_CAPA 8 /* Send REPLCONF capa */
#define COBJ_REPL_RECEIVE_CAPA 9 /* Wait for REPLCONF reply */
#define COBJ_REPL_SEND_PSYNC 10 /* Send PSYNC */
#define COBJ_REPL_RECEIVE_PSYNC 11 /* Wait for PSYNC reply */
/* --- End of handshake states --- */
#define COBJ_REPL_TRANSFER 12 /* Receiving .rdb from master */
#define COBJ_REPL_CONNECTED 13 /* Connected to master */

/* State of slaves from the POV of the master. Used in client->replstate.
 * In SEND_BULK and ONLINE state the slave receives new updates
 * in its output queue. In the WAIT_BGSAVE state instead the server is waiting
 * to start the next background saving in order to send updates to it. */
#define COBJ_REPL_WAIT_BGSAVE_START 14 /* We need to produce a new RDB file. */
#define COBJ_REPL_WAIT_BGSAVE_END 15 /* Waiting RDB file creation to finish. */
#define COBJ_REPL_SEND_BULK 16 /* Sending RDB file to slave. */
#define COBJ_REPL_ONLINE 17 /* RDB file transmitted, sending just updates. */

/* Slave capabilities. */
#define SLAVE_CAPA_NONE 0
#define SLAVE_CAPA_EOF (1<<0)   /* Can parse the RDB EOF streaming format. */

/* Synchronous read timeout - slave side */
#define COBJ_REPL_SYNCIO_TIMEOUT 5

/* List related stuff */
#define COBJ_HEAD 0
#define COBJ_TAIL 1

/* Sort operations */
#define COBJ_SORT_GET 0
#define COBJ_SORT_ASC 1
#define COBJ_SORT_DESC 2
#define COBJ_SORTKEY_MAX 1024

/* Log levels */
#define COBJ_DEBUG 0
#define COBJ_VERBOSE 1
#define COBJ_NOTICE 2
#define COBJ_WARNING 3
#define COBJ_LOG_RAW (1<<10) /* Modifier to log without timestamp */
#define COBJ_DEFAULT_VERBOSITY COBJ_NOTICE

/* Anti-warning macro... */
#define COBJ_NOTUSED(V) ((void) V)

#define ZSKIPLIST_MAXLEVEL 32 /* Should be enough for 2^32 elements */
#define ZSKIPLIST_P 0.25      /* Skiplist P = 1/4 */

/* Append only defines */
#define AOF_FSYNC_NO 0
#define AOF_FSYNC_ALWAYS 1
#define AOF_FSYNC_EVERYSEC 2
#define COBJ_DEFAULT_AOF_FSYNC AOF_FSYNC_EVERYSEC

/* Zip structure related defaults */
#define COBJ_HASH_MAX_ZIPLIST_ENTRIES 512
#define COBJ_HASH_MAX_ZIPLIST_VALUE 64
#define COBJ_LIST_MAX_ZIPLIST_ENTRIES 512
#define COBJ_LIST_MAX_ZIPLIST_VALUE 64
#define COBJ_SET_MAX_INTSET_ENTRIES 512
#define COBJ_ZSET_MAX_ZIPLIST_ENTRIES 128
#define COBJ_ZSET_MAX_ZIPLIST_VALUE 64

/* HyperLogLog defines */
#define COBJ_DEFAULT_HLL_SPARSE_MAX_BYTES 3000

/* Sets operations codes */
#define COBJ_OP_UNION 0
#define COBJ_OP_DIFF 1
#define COBJ_OP_INTER 2

/* Redis maxmemory strategies */
#define COBJ_MAXMEMORY_VOLATILE_LRU 0
#define COBJ_MAXMEMORY_VOLATILE_TTL 1
#define COBJ_MAXMEMORY_VOLATILE_RANDOM 2
#define COBJ_MAXMEMORY_ALLKEYS_LRU 3
#define COBJ_MAXMEMORY_ALLKEYS_RANDOM 4
#define COBJ_MAXMEMORY_NO_EVICTION 5
#define COBJ_DEFAULT_MAXMEMORY_POLICY COBJ_MAXMEMORY_VOLATILE_LRU

/* Scripting */
#define COBJ_LUA_TIME_LIMIT 5000 /* milliseconds */

/* Units */
#define UNIT_SECONDS 0
#define UNIT_MILLISECONDS 1

/* SHUTDOWN flags */
#define COBJ_SHUTDOWN_SAVE 1       /* Force SAVE on SHUTDOWN even if no save
                                       points are configured. */
#define COBJ_SHUTDOWN_NOSAVE 2     /* Don't SAVE on SHUTDOWN. */

/* Command call flags, see call() function */
#define COBJ_CALL_NONE 0
#define COBJ_CALL_SLOWLOG 1
#define COBJ_CALL_STATS 2
#define COBJ_CALL_PROPAGATE 4
#define COBJ_CALL_FULL (COBJ_CALL_SLOWLOG | COBJ_CALL_STATS | COBJ_CALL_PROPAGATE)

/* Command propagation flags, see propagate() function */
#define COBJ_PROPAGATE_NONE 0
#define COBJ_PROPAGATE_AOF 1
#define COBJ_PROPAGATE_REPL 2

/* RDB active child save type. */
#define COBJ_RDB_CHILD_TYPE_NONE 0
#define COBJ_RDB_CHILD_TYPE_DISK 1     /* RDB is written to disk. */
#define COBJ_RDB_CHILD_TYPE_SOCKET 2   /* RDB is written to slave socket. */

/* Keyspace changes notification classes. Every class is associated with a
 * character for configuration purposes. */
#define COBJ_NOTIFY_KEYSPACE (1<<0)    /* K */
#define COBJ_NOTIFY_KEYEVENT (1<<1)    /* E */
#define COBJ_NOTIFY_GENERIC (1<<2)     /* g */
#define COBJ_NOTIFY_STRING (1<<3)      /* $ */
#define COBJ_NOTIFY_LIST (1<<4)        /* l */
#define COBJ_NOTIFY_SET (1<<5)         /* s */
#define COBJ_NOTIFY_HASH (1<<6)        /* h */
#define COBJ_NOTIFY_ZSET (1<<7)        /* z */
#define COBJ_NOTIFY_EXPIRED (1<<8)     /* x */
#define COBJ_NOTIFY_EVICTED (1<<9)     /* e */
#define COBJ_NOTIFY_ALL (COBJ_NOTIFY_GENERIC | COBJ_NOTIFY_STRING | COBJ_NOTIFY_LIST | COBJ_NOTIFY_SET | COBJ_NOTIFY_HASH | COBJ_NOTIFY_ZSET | COBJ_NOTIFY_EXPIRED | COBJ_NOTIFY_EVICTED)      /* A */

/* Get the first bind addr or NULL */
#define COBJ_BIND_ADDR (server.bindaddr_count ? server.bindaddr[0] : NULL)

/* Using the following macro you can run code inside serverCron() with the
 * specified period, specified in milliseconds.
 * The actual resolution depends on server.hz. */
#define run_with_period(_ms_) if ((_ms_ <= 1000/server.hz) || !(server.cronloops%((_ms_)/(1000/server.hz))))

/* We can print the stacktrace, so our assert is defined this way: */
#define redisAssertWithInfo(_c,_o,_e) ((_e)?(void)0 : (_redisAssertWithInfo(_c,_o,#_e,__FILE__,__LINE__),_exit(1)))
#define redisAssert(_e) ((_e)?(void)0 : (_redisAssert(#_e,__FILE__,__LINE__),_exit(1)))
#define redisPanic(_e) _redisPanic(#_e,__FILE__,__LINE__),_exit(1)
#define COBJ_FAKE_CLIENT_FD -1

/*-----------------------------------------------------------------------------
 * Data types
 *----------------------------------------------------------------------------*/

/* A redis object, that is a type able to hold a string / list / set */

/* The actual Redis Object */
#define COBJ_LRU_BITS 24
#define COBJ_LRU_CLOCK_MAX ((1<<COBJ_LRU_BITS)-1) /* Max value of obj->lru */
#define COBJ_LRU_CLOCK_RESOLUTION 1 /* LRU clock resolution in seconds */
typedef struct redisObject {
    unsigned type:4;
    unsigned encoding:4;
    unsigned lru:COBJ_LRU_BITS; /* lru time (relative to server.lruclock) */
    int refcount;
    void *ptr;
} cobj_t;

/* Macro used to initialize a Redis object allocated on the stack.
 * Note that this macro is taken near the structure definition to make sure
 * we'll update it when the structure is changed, to avoid bugs like
 * bug #85 introduced exactly in this way. */
#define initStaticStringObject(_var,_ptr) do { \
    _var.refcount = 1; \
    _var.type = COBJ_STRING; \
    _var.encoding = COBJ_ENCODING_RAW; \
    _var.ptr = _ptr; \
} while(0);

static inline void cobj_init(cobj_t *co, int type, int encoding, void *ptr){
    co->type = type;
    co->encoding = encoding;
    co->ptr = ptr;
    co->refcount = 1;
}



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


#endif  // __COBJ_H__

