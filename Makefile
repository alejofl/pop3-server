CFLAGS=-g -Wall -Wextra -pedantic -pedantic-errors -std=c11 -O3 -D_POSIX_C_SOURCE=200112L -D__BSD_VISIBLE=1 -Wno-unused-parameter -Wno-implicit-fallthrough -Wno-stringop-truncation -fsanitize=address

LDFLAGS=

LIB_HEADERS=src/lib/include

SERVER_SOURCES=$(wildcard src/lib/*.c) $(wildcard src/server/*.c) $(wildcard src/server/client/*.c) $(wildcard src/server/pop3/*.c)
CLIENT_SOURCES=$(wildcard src/lib/*.c) $(wildcard src/client/*.c)

SERVER_OBJECTS=$(SERVER_SOURCES:src/%.c=obj/%.o)
CLIENT_OBJECTS=$(CLIENT_SOURCES:src/%.c=obj/%.o)

OUTPUT_FOLDER=./bin
OBJECTS_FOLDER=./obj

SERVER_OUTPUT_FILE=$(OUTPUT_FOLDER)/turtle-pop3
CLIENT_OUTPUT_FILE=$(OUTPUT_FOLDER)/turtle-client

all: server client

server: $(SERVER_OUTPUT_FILE)

client: $(CLIENT_OUTPUT_FILE)

$(SERVER_OUTPUT_FILE): $(SERVER_OBJECTS)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(LIB_HEADERS) $(LDFLAGS) $(SERVER_OBJECTS) -o $(SERVER_OUTPUT_FILE)

$(CLIENT_OUTPUT_FILE): $(CLIENT_OBJECTS)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(LIB_HEADERS) $(LDFLAGS) $(CLIENT_OBJECTS) -o $(CLIENT_OUTPUT_FILE)

obj/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(LIB_HEADERS) -c $< -o $@

clean:
	rm -rf $(OUTPUT_FOLDER)
	rm -rf $(OBJECTS_FOLDER)

.PHONY: all server client clean