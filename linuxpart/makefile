CC=gcc
CFLAGS=-xc
OFLAGS=-lm
SRC=src/
BIN=bin/

buildclient: $(SRC)client.c
	$(CC) $(CFLAGS) $(SRC)client.c -o $(BIN)client

buildserver: $(SRC)server.c
	$(CC) $(CFLAGS) $(SRC)server.c $(OFLAGS) -o $(BIN)server

build: buildserver buildclient

clean:
	cd $(BIN) && rm server client && cd ..

rebuild: clean build

runserver: $(BIN)server
	$(BIN)server

runclient: $(BIN)client
	$(BIN)client

buildruns: buildserver runserver

buildrunc: buildclient runclient


