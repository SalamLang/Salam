INPUT_FILE = ../test/input.s

CC = gcc
CFLAGS = 
# -Wall -Wextra -std=c11 -g

TARGET = main

SRCS = log.c file.c memory.c array.c lexer.c main.c

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
