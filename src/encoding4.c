#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

void print_utf8_characters(FILE *file)
{
	int ch;
	while ((ch = fgetc(file)) != EOF) {
		unsigned char utf8_char[5] = {0};
		utf8_char[0] = ch;

		int num_bytes = 0;
		if ((ch & 0x80) == 0) {
			num_bytes = 1;
		} else if ((ch & 0xE0) == 0xC0) {
			num_bytes = 2;
		} else if ((ch & 0xF0) == 0xE0) {
			num_bytes = 3;
		} else if ((ch & 0xF8) == 0xF0) {
			num_bytes = 4;
		} else {
			fprintf(stderr, "Invalid UTF-8 encoding detected.\n");
			continue;
		}

		for (int i = 1; i < num_bytes; ++i) {
			int next_ch = fgetc(file);
			if (next_ch == EOF || (next_ch & 0xC0) != 0x80) {
				fprintf(stderr, "Invalid UTF-8 sequence detected.\n");
				exit(EXIT_FAILURE);
			}
			utf8_char[i] = next_ch;
		}

		printf("%s\n", utf8_char);
	}
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return EXIT_FAILURE;
	}

	setlocale(LC_ALL, "");

	FILE *file = fopen(argv[1], "rb");
	if (file == NULL) {
		perror("Error opening file");
		return EXIT_FAILURE;
	}

	print_utf8_characters(file);

	fclose(file);

	return EXIT_SUCCESS;
}
