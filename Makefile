OUTDIR := build
SRCDIR := src
SCRIPTDIR := scripts
TARGET := $(OUTDIR)/10cc
CFLAGS := -std=c11 -g -static

SRCS := $(wildcard $(SRCDIR)/*.c)
HEADERS := $(wildcard $(SRCDIR)/*.h)
OBJS := $(patsubst %.c,$(OUTDIR)/%.o,$(notdir $(SRCS)))
$(warning $(OBJS))

.PHONY: all test clean
all: $(TARGET)

test: $(TARGET)
	$(SCRIPTDIR)/test.sh

# build executable
$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

# compile C sources
$(OUTDIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -r $(OUTDIR)/*
