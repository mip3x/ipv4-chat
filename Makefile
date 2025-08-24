CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -g
LDFLAGS = -pthread

SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

SRC = $(shell find $(SRC_DIR) -name '*.c')

OBJ = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC))

TARGET = ipv4-chat

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

