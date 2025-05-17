CC := gcc
CFLAGS := -Wall -Wextra -pedantic -std=c99 -g -I src/
SRC_DIR := src
BUILD_DIR := build
TARGET := salam
BIN_TARGET := $(BUILD_DIR)/$(TARGET).bin
INPUT_FILE := src/input.salam

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

bin: $(BIN_TARGET)

$(BIN_TARGET): $(BUILD_DIR)/$(TARGET)
	objcopy -O binary $< $@

clean:
	rm -rf $(BUILD_DIR)

run: all
	valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(BUILD_DIR)/$(TARGET) $(INPUT_FILE)

test: bin
	@echo "Running test with $(INPUT_FILE)..."
	./$(BUILD_DIR)/$(TARGET) $(INPUT_FILE)

.PHONY: all clean run bin test
