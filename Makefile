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
SRC := $(shell find $(SRC_DIR) -name '*.c')
SRC := $(subst $(SRC_DIR)/compress.c,, $(SRC))
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
EXE := compress

# We compile all the objects
$(OBJ_DIR)/%.o: $(SRC)
	$(CC) $(FLAGS) -c -o $@ $<

# We compile the main program
$(EXE): $(SRC_DIR)/compress.c $(OBJ) 
	$(CC) $(FLAGS) -o $@ $^

echo:
	echo $(OBJ)
	echo $(SRC)

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o *~ $(EXE)
