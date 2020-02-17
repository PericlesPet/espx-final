SHELL := /bin/bash

RM = rm -f

SRC = src
BIN = bin
EXEC = espx-final

# https://stackoverflow.com/questions/13358131/one-makefile-for-two-compilers
ifeq ($(TARGET),cross)
        CC = ~/cross-pi-gcc-9.2.0-2/bin/arm-linux-gnueabihf-gcc
else
        CC = gcc
endif

all: $(EXEC)

%: $(SRC)/%.c
	$(CC) $< -o $(BIN)/$@ -lpthread

clean:
	$(RM) $(SRC)/*~ *~
