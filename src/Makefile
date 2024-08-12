INPUT_FILE = ../test/input.s

CC = gcc
CFLAGS = -g -fsanitize=undefined,address -Walloca -Wextra -Wall -std=c11

TARGET = main

SRCS = log.c file.c memory.c array.c parser.c generator.c ast.c hashmap.c lexer.c main.c

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
