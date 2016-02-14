
CC := gcc

CFLAGS := -Wall -O2 -DDEBUG

LDFLAGS := 

objs := zmalloc.o logger.o sds.o dict.o  cobj.o cache.o

cache-test-objs := cache-test.o


%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<


all : $(objs)
	$(CC)  -o cache $(objs) $(LDFLAGS)


clean :
	rm -f $(objs) cache

cache-test : $(objs) $(cache-test-objs)
	$(CC) -o cache-test $(objs) $(cache-test-objs) $(LDFLAGS)




