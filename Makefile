CC = gcc
CFLAGS = -Wall -Wextra -I/usr/local/include/cjson -Iinclude -I/opt/homebrew/Cellar/libarchive/3.7.7/include/
LDFLAGS = -lcurl -lcjson -larchive -pthread -L/usr/local/lib -L/opt/homebrew/Cellar/libarchive/3.7.7/lib/

SRC = src/main.c src/parser.c ./src/network.c ./src/json_parser.c ./src/downloader.c ./src/package_manager.c ./src/thread_pool.c ./src/logger.c
OBJ = $(SRC:src/%.c=build/%.o)
BIN = npm_c_manager

all: build $(BIN)

build:
	mkdir -p build

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)
	install_name_tool -add_rpath /usr/local/lib $@


build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build $(BIN) node_modules


install:
	./npm_c_manager install axios


uninstall:
	./npm_c_manager uninstall axios


.PHONY: all clean
