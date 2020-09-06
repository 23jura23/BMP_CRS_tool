LAB_NAME = bmp_tool

#######################################################

.PHONY: all build debug release fsanitize test clean

CFLAGS = -Wall -Wextra -Werror
INC = -Iinclude
EXTRA_FLAGS = 

DEBUG_FLAGS = -g
RELEASE_FLAGS = -Ofast
FS_FLAGS = ${DEBUG_FLAGS} -fsanitize=address

FILES = $(shell ls src)
SRCS = $(FILES:%.c=src/%.c)
OBJS = $(FILES:%.c=obj/%.o)
DEP = $(OBJS:%.o=%.d)
BIN = ${LAB_NAME}

all: release
build: bin/${BIN}

debug: EXTRA_FLAGS += ${DEBUG_FLAGS}
debug: build

release: EXTRA_FLAGS += ${RELEASE_FLAGS}
release: build

fsanitize: EXTRA_FLAGS += ${FS_FLAGS}
fsanitize: build

test: build
	./bin/${BIN}

bin/${BIN}: ${OBJS} | obj bin
	gcc ${CFLAGS} ${EXTRA_FLAGS} ${OBJS} -o bin/${BIN}

-include ${DEP}

obj/%.o: src/%.c | obj
	gcc ${CFLAGS} ${EXTRA_FLAGS} ${INC} -MMD -c $< -o $@

obj:
	mkdir obj

bin:
	mkdir bin

clean:
	rm -rf obj bin

