# We set the environment
SHELL := /bin/sh

# We define the compiler we will use
CC := gcc

# We define the organization of our directory
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := .

# We define the executables
COMP := compress
DECOMP := decompress
EXE = $(COMP) $(DECOMP)

$(COMP): $(SRC_DIR)/main.c 
	$(CC) $^ -o $@

clean:
	rm -f $(OBJ_DIR)/*.o *~ $(EXE)
