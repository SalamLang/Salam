CC = gcc
CFLAGS = -std=c11 -Wall -O3
DEBUG_FLAGS = -g -Walloca -Wextra -fsanitize=address,undefined
TARGET = salam
OUTPUT_DIR = out/
INPUT_FILE = example/input.salam
SRCS = src/log.c src/file.c src/memory.c src/array.c src/downloader.c src/parser.c src/parser_layout.c src/generator.c src/generator_layout.c src/generator_salam.c src/generator_layout_style.c src/generator_identifier.c src/string_buffer.c src/validator.c src/hashmap.c src/hashmap_custom.c src/array_custom.c src/lexer.c src/ast.c src/ast_layout.c src/ast_layout_style.c src/main.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

debug: CFLAGS = -std=c11 -Wall $(DEBUG_FLAGS)
debug: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: run
run: $(TARGET)
	@if [ ! -d $(OUTPUT_DIR) ]; then \
		mkdir -p $(OUTPUT_DIR); \
		echo "Created directory $(OUTPUT_DIR)"; \
	fi
	./$(TARGET) $(INPUT_FILE) $(OUTPUT_DIR)

.PHONY: clean
clean:
	rm -rf $(OUTPUT_DIR)
	rm -f $(OBJS) $(TARGET)

.PHONY: check
check: checkinstall
	@if ! command -v pre-commit >/dev/null 2>&1; then \
		echo "Error: pre-commit is not installed. Please install it first."; \
		exit 1; \
	fi
	pre-commit run --all-files

.PHONY: checkinstall
checkinstall:
	@if ! command -v pre-commit >/dev/null 2>&1; then \
		echo "Error: pre-commit is not installed. Please install it first."; \
		exit 1; \
	fi
	pre-commit install

.PHONY: checkupdate
checkupdate: checkinstall
	@if ! command -v pre-commit >/dev/null 2>&1; then \
		echo "Error: pre-commit is not installed. Please install it first."; \
		exit 1; \
	fi
	pre-commit autoupdate

.PHONY: install
install:
	@if ! command -v pip >/dev/null 2>&1; then \
		echo "Error: pip is not installed. Please install it first."; \
		exit 1; \
	fi
	pip install -r requirements.txt
