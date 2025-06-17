# =========================
# Makefile for Emscripten
# =========================

# Output folder and target name
TARGET_DIR = Emiscripten
TARGET = $(TARGET_DIR)/index

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
CFLAGS_CPP = $(INCLUDE) -std=c++17
CFLAGS_C = $(INCLUDE)

LINK_FLAGS = \
-s USE_WEBGL2=1 \
-s USE_GLFW=3 \
--preload-file $(TARGET_DIR)/vShader.txt \
--preload-file $(TARGET_DIR)/fShader.txt

# Object files
OBJ_CPP = $(SRC_CPP:.cpp=.o)
OBJ_C = $(SRC_C:.c=.o)

# ===== Targets =====

# Default build target
all: prepare $(TARGET).html

# Create output directory and copy shader files
prepare:
	mkdir -p $(TARGET_DIR)
	cp Shaders/vShader.txt Shaders/fShader.txt $(TARGET_DIR)

# Compile C++ source files
%.o: %.cpp
	$(CC) $(CFLAGS_CPP) -c $< -o $@

# Compile C source files
%.o: %.c
	$(CC) $(CFLAGS_C) -c $< -o $@

# Link objects into WebAssembly/HTML output
$(TARGET).html: $(OBJ_CPP) $(OBJ_C)
	$(CC) $^ -o $@ $(LINK_FLAGS)

# Clean build files
clean:
	rm -f *.o
	rm -rf $(TARGET_DIR)
