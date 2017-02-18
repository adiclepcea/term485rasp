IDIR=./include
SDIR=./src
TDIR=./test
OUTPUT=reader
TESTS=tests
CC=gcc
CFLAGS=-I$(IDIR) -lpthread -lcurl -Wall -pedantic -std=gnu99
LIBDIR=./lib/
SRC=$(SDIR)/*.c ./lib/libcobserver.a

.PHONY: build clean help test buildhb

help:
	@echo "make build   - build the program"
	@echo "make clean   - clean the generated files"
	@echo "make help    - show this help message"

build:
	@$(CC) -o $(OUTPUT) $(SRC) $(CFLAGS)

test:
	@echo "Building tests"
	@$(CC) -o $(TESTS) $(TDIR)/*.c $(SDIR)/reader.c $(SDIR)/readerMock.c -I$(TDIR) $(CFLAGS)
	@echo "Runing tests"
	@./$(TESTS)

clean:
	@rm $(OUTPUT)
	@rm $(TESTS)
	@rm hb
buildhb:
	go build -o hb main.go

docker: clean-docker build buildhb
	docker build -t esensesclient .

clean-docker:
	exists=$(docker images | grep esensesclient || true)
	[ ! -z "${exists}" ] && { docker rmi -f esensesclient; } || true
