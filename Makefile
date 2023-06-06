CFLAGS=-g -Wall -Wextra -pedantic -pedantic-errors -O3 -std=c11 -D_POSIX_C_SOURCE=200112L -D__BSD_VISIBLE=1 -Wno-unused-parameter -Wno-implicit-fallthrough  -fsanitize=address

LDFLAGS=

SOURCES=$(wildcard src/*.c)

OBJECTS=$(SOURCES:src/%.c=obj/%.o)

OUTPUT_FOLDER=./bin
OBJECTS_FOLDER=./obj

OUTPUT_FILE=$(OUTPUT_FOLDER)/pop3-server

all: server

server: $(OUTPUT_FILE)

$(OUTPUT_FILE): $(OBJECTS)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $(OUTPUT_FILE)

obj/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) $(GCCFLAGS) -c $< -o $@

clean:
	rm -rf $(OUTPUT_FOLDER)
	rm -rf $(OBJECTS_FOLDER)

.PHONY: all server client clean