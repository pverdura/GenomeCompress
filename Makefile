# We set the environment
SHELL := /bin/sh

# We define the organization of our directory
SRC_DIR := ./src
OBJ_DIR := ./obj
INC_DIR := ./include
BIN_DIR := .

# We define the compiler we will use
CC := gcc
FLAGS := -I $(INC_DIR) -Wall

# We define the executable files
EXE_COM := compress
EXE_DEC := decompress
SRC := $(shell find $(SRC_DIR) -name '*.c' | grep -v $(EXE_COM))
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
EXE := $(EXE_COM) $(EXE_DEC)

all: $(EXE)

# We compile all the objects
$(OBJ_DIR)/%.o: $(SRC)
	$(CC) $(FLAGS) -c -o $@ $<

# We compile the main programs
$(EXE_COM): $(SRC_DIR)/$(EXE_COM).c $(OBJ) 
	$(CC) $(FLAGS) -o $@ $^

$(EXE_DEC): $(SRC_DIR)/$(EXE_DEC).c $(OBJ) 
	$(CC) $(FLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o *~ $(EXE)
