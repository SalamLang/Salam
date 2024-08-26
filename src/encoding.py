import sys
import os


def file_appends(path, content):
    try:
        with open(path, "a", encoding="utf-8") as file:
            file.write(content)
        return True
    except IOError as e:
        print(f"Error appending to file: {e}", file=sys.stderr)
        return False


def print_utf8_as_wchars(file_path):
    output_path = "windows-logs.txt"

    try:
        with open(file_path, "rb") as file:
            while True:
                byte = file.read(1)
                if not byte:
                    break

                # Decode the byte into a string
                byte = byte.decode("latin1")  # Use 'latin1' to preserve byte values
                codepoint = ord(byte)

                # Read more bytes if needed
                if 0x80 <= codepoint <= 0xBF:
                    continue  # Continuation bytes are handled by the first byte

                if codepoint <= 0x7F:
                    char = chr(codepoint)
                elif 0xC0 <= codepoint <= 0xDF:
                    char = byte + file.read(1).decode("latin1")
                elif 0xE0 <= codepoint <= 0xEF:
                    char = byte + file.read(2).decode("latin1")
                elif 0xF0 <= codepoint <= 0xF7:
                    char = byte + file.read(3).decode("latin1")
                else:
                    continue

                # Append character to the output file
                if not file_appends(output_path, char):
                    print(f"Failed to append character '{char}'", file=sys.stderr)

                # Print character to the console
                print(char, end="")

    except IOError as e:
        print(f"Error reading file: {e}", file=sys.stderr)
        sys.exit(1)


def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <file_path>", file=sys.stderr)
        sys.exit(1)

    file_path = sys.argv[1]
    print_utf8_as_wchars(file_path)


if __name__ == "__main__":
    main()
