CFLAGS=-g -Wall -Wextra -pedantic -pedantic-errors -std=c11 -O3 -D_POSIX_C_SOURCE=200112L -D__BSD_VISIBLE=1 -Wno-unused-parameter -Wno-implicit-fallthrough -Wno-stringop-truncation -fsanitize=address

LDFLAGS=

LIB_HEADERS=src/lib/include

SOURCES=$(wildcard src/lib/*.c) $(wildcard src/*.c)

OBJECTS=$(SOURCES:src/%.c=obj/%.o)

OUTPUT_FOLDER=./bin
OBJECTS_FOLDER=./obj

OUTPUT_FILE=$(OUTPUT_FOLDER)/pop3-server

all: server

server: $(OUTPUT_FILE)

$(OUTPUT_FILE): $(OBJECTS)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(LIB_HEADERS) $(LDFLAGS) $(OBJECTS) -o $(OUTPUT_FILE)

obj/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(LIB_HEADERS) -c $< -o $@

clean:
	rm -rf $(OUTPUT_FOLDER)
	rm -rf $(OBJECTS_FOLDER)

.PHONY: all server clean