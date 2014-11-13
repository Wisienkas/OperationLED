CC=gcc
CFLAGS=-xc
OFLAGS=-lm
SRC=src/
BIN=bin/

buildclient: $(SRC)soundAssign.c
	$(CC) $(CFLAGS) $(SRC)soundAssign.c -o $(BIN)client

buildserver: $(SRC)soundserver.c
	$(CC) $(CFLAGS) $(SRC)soundserver.c $(OFLAGS) -o $(BIN)server

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


