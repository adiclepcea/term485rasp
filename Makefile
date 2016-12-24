IDIR=./include
SDIR=./src
TDIR=./test
OUTPUT=reader
TESTS=tests
CC=gcc
CFLAGS=-I$(IDIR)
SRC=$(SDIR)/*.c

.PHONY: build clean help test

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
