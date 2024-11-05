INPUT_FILE = ../example/input.salam
INPUT_FILE = ../example/test6.salam
INPUT_FILE = ../example/testhover.salam
INPUT_FILE = ../example/testgroup.salam
INPUT_FILE = ../example/test-include.salam
INPUT_FILE = ../example/repeat-block.salam

OUTPUT_DIR = ../out/

CC = gcc

CFLAGS = -std=c11 -Wall -O3

DEBUG_FLAGS = -g -Walloca -Wextra -fsanitize=address,undefined

TARGET = salam

SRCS = log.c file.c memory.c array.c downloader.c parser.c parser_layout.c generator.c generator_layout.c generator_salam.c generator_layout_style.c generator_identifier.c string_buffer.c validator.c hashmap.c hashmap_custom.c array_custom.c lexer.c ast.c ast_layout.c ast_layout_style.c main.c

OBJS = $(SRCS:.c=.o)
WIN_OBJS = $(SRCS:.c=.wino)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

debug: CFLAGS = -std=c11 -Wall $(DEBUG_FLAGS)
debug: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	@if [ ! -d $(OUTPUT_DIR) ]; then \
		mkdir -p $(OUTPUT_DIR); \
		echo "Created directory $(OUTPUT_DIR)"; \
	fi
	./$(TARGET) $(INPUT_FILE) $(OUTPUT_DIR)

clean:
	rm -rf $(OUTPUT_DIR)
	rm -f $(OBJS) $(WIN_OBJS) $(TARGET)

.PHONY: all clean debug
