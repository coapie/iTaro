
CC := gcc

CFLAGS := -Wall -O2 -DDEBUG -g

LDFLAGS := 

objs := usmm.o usmm_slab.o bitmap.o logger.o

usmm-test-objs := usmm_test.o $(objs)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<


all : $(objs)
	$(CC)  -o usmm $(objs) $(LDFLAGS)


clean :
	rm -f $(objs) usmm
	rm -f core

usmm-test : $(usmm-test-objs)
	$(CC) -o usmm-test $(usmm-test-objs) $(LDFLAGS)




