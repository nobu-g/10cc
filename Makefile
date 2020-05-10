CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

10cc: $(OBJS)
	$(CC) -o 10cc $(OBJS) $(LDFLAGS)

$(OBJS): 10cc.h $(SRCS)

test: 10cc
	./test.sh

clean:
	rm -f 10cc *.o tmp*

.PHONY: test clean
