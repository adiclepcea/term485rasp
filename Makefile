IDIR=./include
SDIR=./src
OUTPUT=reader
CC=gcc
CFLAGS=-I$(IDIR)
SRC=$(SDIR)/*.c

.PHONY: build clean help

help:
	@echo "make build   - build the program"
	@echo "make clean   - clean the generated files"
	@echo "make help    - show this help message"

build:
	@$(CC) -o $(OUTPUT) $(SRC) $(CFLAGS)


clean:
	@rm $(OUTPUT)
