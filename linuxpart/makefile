CC=gcc
CFLAGS=-xc
DFLAGS=-g -Wall
OFLAGS=-lm
SRC=src/
BIN=bin/
DEPLOYBIN=/usr/bin/

buildclient: $(SRC)client.c
	$(CC) $(CFLAGS) $(DFLAGS) $(SRC)client.c -o $(BIN)client

buildserver: $(SRC)server.c
	$(CC) $(CFLAGS) $(DFLAGS) $(SRC)server.c $(OFLAGS) -o $(BIN)server

buildpython: $(SRC)StoreToDb.py
	cp $(SRC)StoreToDb.py $(BIN)StoreToDb.py

build: buildpython buildserver buildclient

deployclient: $(SRC)client.c
	$(CC) $(CFLAGS) $(SRC)client.c -o $(DEPLOYBIN)hwclient

deployserver: $(SRC)server.c
	$(CC) $(CFLAGS) $(SRC)server.c $(OFLAGS) -o $(DEPLOYBIN)hwserver

deploypython: $(SRC)StoreToDb.py
	cp $(SRC)StoreToDb.py $(DEPLOYBIN)StoreToDb.py

deploy: deploypython deployclient deployserver

clean:
	cd $(BIN) && rm server client && cd ..

rebuild: clean build

runserver: $(BIN)server
	$(BIN)server

runclient: $(BIN)client
	$(BIN)client 828 717 999 7 62

buildruns: buildserver runserver

buildrunc: buildclient runclient


