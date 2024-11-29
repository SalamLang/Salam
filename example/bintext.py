import sys


def print_hexdump(filename: str) -> None:
    try:
        with open(filename, "rb") as file:
            offset = 0
            while chunk := file.read(16):
                hex_repr = " ".join(f"{byte:02x}" for byte in chunk)
                ascii_repr = "".join(
                    (chr(byte) if 32 <= byte < 127 else ".") for byte in chunk
                )
                print(f"{offset:08x} {hex_repr.ljust(48)} {ascii_repr}")
                offset += len(chunk)
    except FileNotFoundError:
        print(f"File '{filename}' not found.")
    except Exception as e:
        print(f"An error occurred: {e}")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <filename>")
    else:
        print_hexdump(sys.argv[1])
