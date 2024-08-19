INPUT_FILE = ../test/input.s

CC = gcc
CFLAGS =
# CFLAGS = -g -Walloca -Wextra -Wall -fsanitize=address,undefined
# -std=c11

TARGET = main

SRCS = log.c file.c memory.c array.c parser.c generator.c generator_layout.c generator_identifier.c string.c validator.c hashmap.c lexer.c ast.c ast_layout.c main.c

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
