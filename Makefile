# =========================
# Makefile for Emscripten
# =========================

TARGET = index

# Source files
SRC_CPP = main.cpp
SRC_C = glad.c

# Include paths
INCLUDE = \
-Ifiles/include \
-I/opt/homebrew/include

# Compiler
CC = emcc

# Flags
CFLAGS_C = $(INCLUDE)             # C compile flags (no C++ standard here)
CFLAGS_CPP = $(INCLUDE) -std=c++17  # C++ compile flags

LINK_FLAGS = -s USE_WEBGL2=1 -s USE_GLFW=3 --preload-file vShader.txt --preload-file fShader.txt

# Object files
OBJ_CPP = $(SRC_CPP:.cpp=.o)
OBJ_C = $(SRC_C:.c=.o)

# Build all
all: $(TARGET).html

# Compile C++ source files
%.o: %.cpp
	$(CC) $(CFLAGS_CPP) -c $< -o $@

# Compile C source files
%.o: %.c
	$(CC) $(CFLAGS_C) -c $< -o $@

# Link all objects
$(TARGET).html: $(OBJ_CPP) $(OBJ_C)
	$(CC) $^ -o $@ $(LINK_FLAGS)

# Clean build files
clean:
	rm -f $(OBJ_CPP) $(OBJ_C) $(TARGET).html $(TARGET).js $(TARGET).wasm
