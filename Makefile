CC = gcc
CFLAGS = -std=c11 -Wall -O3
DEBUG_FLAGS = -g -Walloca -Wextra -fsanitize=address,undefined
TARGET = salam
OUTPUT_DIR = out/
INPUT_FILE = example/input.salam
SRCS = src/log.c src/file.c src/memory.c src/array.c src/downloader.c src/parser.c src/parser_layout.c src/generator.c src/generator_layout.c src/generator_salam.c src/generator_layout_style.c src/generator_identifier.c src/string_buffer.c src/validator.c src/validator_style.c src/hashmap.c src/hashmap_custom.c src/array_custom.c src/lexer.c src/ast.c src/ast_layout.c src/ast_layout_style.c src/main.c
OBJS = $(SRCS:.c=.o)

PYTHON = $(shell command -v python || command -v python3 || echo python)

TEST_MAIN_FILE = test/tests.py

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

debug: CFLAGS = -std=c11 -Wall $(DEBUG_FLAGS)
debug: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: test t
test t: $(TARGET)
	@echo "Start testing..."
	$(PYTHON) $(TEST_MAIN_FILE)
	@echo "End testing"

.PHONY: run r
run r: $(TARGET)
	@if [ ! -d $(OUTPUT_DIR) ]; then \
		mkdir -p $(OUTPUT_DIR); \
		echo "Created directory $(OUTPUT_DIR)"; \
	fi
	./$(TARGET) $(INPUT_FILE) $(OUTPUT_DIR)

.PHONY: clean c
clean c:
	rm -rf $(OUTPUT_DIR)
	rm -f $(OBJS) $(TARGET)

.PHONY: check ck
check ck: checkinstall
	@if ! command -v pre-commit >/dev/null 2>&1; then \
		echo "Error: pre-commit is not installed. Please install it first."; \
		exit 1; \
	fi
	pre-commit run --all-files

.PHONY: checkinstall cki
checkinstall cki:
	@if ! command -v pre-commit >/dev/null 2>&1; then \
		echo "Error: pre-commit is not installed. Please install it first."; \
		exit 1; \
	fi
	pre-commit install

.PHONY: checkupdate cku
checkupdate cku: checkinstall
	@if ! command -v pre-commit >/dev/null 2>&1; then \
		echo "Error: pre-commit is not installed. Please install it first."; \
		exit 1; \
	fi
	pre-commit autoupdate

.PHONY: install i
install i:
	@if ! command -v pip >/dev/null 2>&1; then \
		echo "Error: pip is not installed. Please install it first."; \
		exit 1; \
	fi
	$(PYTHON) -m pip install -r requirements.txt
