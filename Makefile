OUTDIR := build
SRCDIR := src
TARGET := $(OUTDIR)/10cc
CFLAGS := -std=c11 -g -static -Wall

SRCS := $(wildcard $(SRCDIR)/*.c)
HEADERS := $(wildcard $(SRCDIR)/*.h)
OBJS := $(patsubst %.c,$(OUTDIR)/%.o,$(notdir $(SRCS)))
$(warning $(OBJS))

.PHONY: all
all: $(TARGET)

# build executable
$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

# compile C sources
$(OUTDIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: test
test: test/test
	./$<

test/test: test/test.s test/util.o
	$(CC) $(CFLAGS) -o $@ $^

test/test.s: $(TARGET) test/test.c
	$(TARGET) test/test.c > $@

test/test.c: test/prep.sh
	./$< > $@

test/util.o: test/util.c
	$(CC) $(CFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	rm -f $(OUTDIR)/* test/test* test/*.o
