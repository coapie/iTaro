User-mode Shared Memory Management
-----

目前仅支持单线程模式！！

目前仅支持单线程模式！！

目前仅支持单线程模式！！


问题：

解决方案：

通过共享内存，实现相对进程独立的内存管理，在进程重启时，可以快速恢复工作环境；实现此目标需要两个特性：

1. 通过共享内存的方式，将内存映射到相同的地址空间，这样进程重入的时候，所有场景数据的地址才是一致的。当然通过相对地址引用也可以，但是实现起来会比较复杂，不建议使用；
2. 在特定的共享内存位置存储数据的管理信息，这样整理工作环境内存才可以被加载。

usmm模块就是在这种思路下实现的，基本思路：

1. 创建临时文件，并将临时文件mmap到内存的指定位置（64位linux，用户态地址空间中选一段，基本不会碰到冲突的情况），mmap的时候使用MAP_LOCKED的参数，数据不会实际写入到文件，只会在内存中；
2. 对mmap的内存进行切分管理，分为两级：
	1. 第一级实现chunk的管理，默认2MB粒度；
	2. 第二级实现slab管理，采用伙伴算法，在2MB的chunk内切分更小的块，为分41个等级，从8Bytes ~ 128kBytes，划分方法有参考jemalloc.
	
	为了达到高效简单，第一级采用freelist + bitmap方式，freelist用于分配、释放时的快速管理O(1)级复杂度；bitmap用于分配超过2MB的连续空间时使用。在cache场景里，只有dict的hashtable需要大块的连续内存，是小概率使用场景，因此没有使用复杂的数据结构。 
3. 在usmm的头部预留2MB的空间，用于存储自身的管理信息与开放给上层应用存储起始点信息。

主要接口：

```
// 初始化 usmm模块，参数由conf输入；
int usmm_init(usmm_conf_t *conf,  usmm_t **us);
int usmm_fini(usmm_t *us);

// 大块内存分配接口，最小粒度为2MB-8Byes,
// 可以分配usmm管理的内存中的最大连续块，
// 使用大内存的场景建议初始化的时候一次分配充分；
void *usmm_heap_alloc(usmm_t *us, size_t size);
void usmm_heap_free(usmm_t *us, void *ptr);

// slab 内存管理器，分配的内存大小为 8 Bytes ~ 128k Bytes；
void *usmm_slice_alloc(usmm_t *us, size_t size);
void  usmm_slice_free(usmm_t *us, void *ptr);

// 存储应用数据起始指针
void *usmm_slot_set(usmm_t *us, int slot, void *ptr);
void *usmm_slot_get(usmm_t *us, int slot);
int usmm_slot_append(usmm_t *us, void *ptr);

void usmm_stat(usmm_t *us);
```

