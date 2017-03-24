.PHONY: all clean test lib debug

SRC_DIR = src
INCLUDE_DIRS =
OBJECT_DIR = obj
LIBRARY_DIRS = -Lbin
BIN_DIR = bin

OBJ_FILES = midi.o 
RUN_OBJ_FILES = test.o
BIN_NAME = midi
LIBRARIES = -lmidi

OBJ = $(patsubst %, $(OBJECT_DIR)/%,$(OBJ_FILES))
RUN_OBJ = $(patsubst %, $(OBJECT_DIR)/%,$(RUN_OBJ_FILES))

BIN = $(BIN_DIR)/$(BIN_NAME)
LIB = $(BIN_DIR)/lib$(BIN_NAME).a
RUN_ARGS = 

CXX = gcc
CXXFLAGS = -std=c99 $(INCLUDES) -Wall -g -pedantic
LINKFLAGS = $(LIBRARY_DIRS) $(LIBRARIES)

AR = ar
ARFLAGS = rvs

all: lib

$(OBJECT_DIR):
	mkdir $(OBJECT_DIR)

$(BIN_DIR):
	mkdir $(BIN_DIR)

$(SRC_DIR):
	mkdir $(SRC_DIR)

$(OBJECT_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJECT_DIR)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(BIN): $(RUN_OBJ) $(LIB) | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LINKFLAGS)

$(LIB): $(OBJ) | $(BIN_DIR)
	$(AR) $(ARFLAGS) $@ $^

lib: $(LIB)

test: $(BIN)
	$(BIN) $(RUN_ARGS)

debug: $(BIN)
	gdb $(BIN)

clean:
	rm -rf obj
	rm -rf bin
