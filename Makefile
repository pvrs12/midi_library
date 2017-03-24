SRC_DIR = src
INCLUDE_DIRS =
OBJECT_DIR = obj
LIBRARY_DIRS = -Llib
BIN_DIR = bin
LIB_DIR = lib

OBJ_FILES = midi.o 
RUN_OBJ_FILES = test.o
NAME = midi
LIBRARIES = -lmidi

OBJ = $(patsubst %, $(OBJECT_DIR)/%,$(OBJ_FILES))
RUN_OBJ = $(patsubst %, $(OBJECT_DIR)/%,$(RUN_OBJ_FILES))

BIN = $(BIN_DIR)/$(NAME)
LIB = $(LIB_DIR)/lib$(NAME).a
RUN_ARGS = 

CXX = gcc
CXXFLAGS = -std=c99 $(INCLUDES) -Wall -g -pedantic
LINKFLAGS = $(LIBRARY_DIRS) $(LIBRARIES)

AR = ar
ARFLAGS = rvs

.PHONY: all
all: library

$(OBJECT_DIR):
	mkdir $(OBJECT_DIR)

$(LIB_DIR):
	mkdir $(LIB_DIR)

$(BIN_DIR):
	mkdir $(BIN_DIR)

$(SRC_DIR):
	mkdir $(SRC_DIR)

$(OBJECT_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJECT_DIR)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(BIN): $(RUN_OBJ) $(LIB) | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LINKFLAGS)

$(LIB): $(OBJ) | $(LIB_DIR)
	$(AR) $(ARFLAGS) $@ $^

.PHONY: library
library: $(LIB)

.PHONY: test
test: $(BIN)
	$(BIN) $(RUN_ARGS)

.PHONY: debug
debug: $(BIN)
	gdb $(BIN)

.PHONY: clean
clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(BIN_DIR)
	rm -rf $(LIB_DIR)
