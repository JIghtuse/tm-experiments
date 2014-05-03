CC := gcc-4.8
MKDIR := mkdir -p
CFLAGS += -Wall -Werror -Wextra -pedantic -D_GNU_SOURCE -fgnu-tm -g
CFLAGS += -D_USE_TSX -DFAKE_BMAP_SIZE=10000000
LDFLAGS := -pthread
PROGS := bin/hist
OBJS := $(patsubst src/%.c,obj/%.o, $(wildcard src/*.c))

.PHONY: all clean

all: build

build: $(PROGS)

clean:
	rm -rf $(PROGS) $(OBJS) obj/*.d

bin/hist: obj/hist.o obj/bmp.o obj/files.o obj/hpctimer.o

$(PROGS):
	@$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

obj/%.o : src/%.c
	@$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) -c -MD -o $@ $<

