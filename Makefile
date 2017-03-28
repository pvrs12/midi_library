SRC_DIR = src
INCLUDE_DIRS =
OBJECT_DIR = obj
LIBRARY_DIRS = -Llib
BIN_DIR = bin
LIB_DIR = lib
INC_DIR = include

OBJ_FILES = midi.o midi_helper.o
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

$(INC_DIR):
	mkdir $(INC_DIR)

$(SRC_DIR):
	mkdir $(SRC_DIR)

$(OBJECT_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJECT_DIR)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(BIN): $(RUN_OBJ) $(LIB) | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LINKFLAGS)

$(LIB): $(OBJ) | $(LIB_DIR) $(INC_DIR)
	$(AR) $(ARFLAGS) $@ $^
	cp $(SRC_DIR)/*.h $(INC_DIR)

.PHONY: library
library: $(LIB)

.PHONY: test
test: $(BIN)
	$(BIN) $(RUN_ARGS)

.PHONY: debug
debug: $(BIN)
	gdb $(BIN)

.PHONY: valgrind
valgrind: $(BIN)
	valgrind -v --leak-check=full $(BIN)

.PHONY: clean
clean:
	rm -rf $(OBJECT_DIR)
	rm -rf $(BIN_DIR)
	rm -rf $(LIB_DIR)
	rm -rf $(INC_DIR)
