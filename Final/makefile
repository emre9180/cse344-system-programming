# Compiler
CC := gcc

# Directories
SRC_DIR := src
OBJ_DIR := O_FILES

# Source files
SERVER_SRC_FILES := $(wildcard $(SRC_DIR)/Server/*.c)
CLIENT_SRC_FILES := $(wildcard $(SRC_DIR)/Client/*.c)

# Object files
SERVER_OBJ_FILES := $(patsubst $(SRC_DIR)/Server/%.c,$(OBJ_DIR)/%.o,$(SERVER_SRC_FILES))
CLIENT_OBJ_FILES := $(patsubst $(SRC_DIR)/Client/%.c,$(OBJ_DIR)/%.o,$(CLIENT_SRC_FILES))

# Compiler flags
CFLAGS := -Wall -Wextra -pthread -lrt

# Target executables
TARGET := PideHouse
CLIENT_TARGET := HungryVeryMuch

# Default target
all: $(TARGET) $(CLIENT_TARGET)

# Rule to build the PideHouse executable
$(TARGET): $(SERVER_OBJ_FILES)
	$(CC) $(CFLAGS) $^ -lm -o $@

# Rule to build the HungryVeryMuch executable
$(CLIENT_TARGET): $(CLIENT_OBJ_FILES)
	$(CC) $(CFLAGS) $^ -o $@

# Rule to build object files for the server
$(OBJ_DIR)/%.o: $(SRC_DIR)/Server/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to build object files for the client
$(OBJ_DIR)/%.o: $(SRC_DIR)/Client/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to clean the project
clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET) $(CLIENT_TARGET)
