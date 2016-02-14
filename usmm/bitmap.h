
#ifndef __BITMAP_H__
#define __BITMAP_H__

#ifdef __cplusplus
extern "C"{
#endif

void bitmap_set(void *bitmap, int index);
void bitmap_clr(void *bitmap, int index);
void bitmap_set_range(void *bitmap, int start, int end);
void bitmap_clr_range(void *bitmap, int start, int end);
int  bitmap_find_range(void *bitmap, int range, int max);
int  bitmap_test(void *bitmap, int index);

#ifdef __cplusplus
}
#endif

#endif // __BITMAP_H__

