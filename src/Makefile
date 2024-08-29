INPUT_FILE = ../example/input.s
INPUT_FILE = ../example/test6.salam
INPUT_FILE = ../example/testhover.salam
INPUT_FILE = ../example/testgroup.salam

CC = gcc
CFLAGS =
# CFLAGS = -g -Walloca -Wextra -Wall -fsanitize=address,undefined -std=c11

TARGET = main

SRCS = log.c file.c memory.c array.c parser.c parser_layout.c generator.c generator_layout.c generator_layout_style.c generator_identifier.c string.c validator.c hashmap.c hashmap_custom.c array_custom.c lexer.c ast.c ast_layout.c ast_layout_style.c main.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) $(INPUT_FILE)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
