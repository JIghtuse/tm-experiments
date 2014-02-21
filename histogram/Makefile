CC := gcc
MKDIR := mkdir -p
CFLAGS := -Wall -Werror -Wextra -pedantic -D_GNU_SOURCE -fgnu-tm -g
LDFLAGS := -pthread
PROGS := bin/hist
OBJS := $(patsubst src/%.c,obj/%.o, $(wildcard src/*.c))

.PHONY: all clean

all: build

build: $(PROGS)

clean:
	rm -rf $(PROGS) $(OBJS)

bin/hist: obj/hist.o obj/bmp.o obj/files.o

$(PROGS):
	@$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

obj/%.o : src/%.c
	@$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) -c -MD -o $@ $<

