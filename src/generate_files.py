import os
import sys

def create_header_file(c_file_path):
    base_name = os.path.splitext(os.path.basename(c_file_path))[0]
    h_file_path = os.path.join(os.path.dirname(c_file_path), base_name + ".h")

    if os.path.exists(h_file_path):
        print(f"[SKIP] Header already exists: {h_file_path}")
        return

    guard = base_name.upper() + "_H"
    content = f"#ifndef {guard}\n#define {guard}\n\n// Declarations for {base_name}.c\n\n#endif // {guard}\n"

    with open(h_file_path, "w") as h_file:
        h_file.write(content)

    print(f"[CREATE] Header: {h_file_path}")

def create_c_and_h_files_for_empty_directory(directory):
    if not os.path.isdir(directory):
        print(f"[SKIP] Not a directory: {directory}")
        return
    if os.listdir(directory):
        print(f"[SKIP] Not empty: {directory}")
        return

    base_name = os.path.basename(os.path.abspath(directory))
    c_path = os.path.join(directory, base_name + ".c")
    h_path = os.path.join(directory, base_name + ".h")

    with open(c_path, "w") as c_file:
        c_file.write(f'#include "{base_name}.h"\n\n// Implementation here\n')
    print(f"[CREATE] C File: {c_path}")

    guard = base_name.upper() + "_H"
    with open(h_path, "w") as h_file:
        h_file.write(f"#ifndef {guard}\n#define {guard}\n\n// Declarations here\n\n#endif // {guard}\n")
    print(f"[CREATE] Header: {h_path}")

def scan_directory(root):
    for dirpath, dirnames, filenames in os.walk(root):
        for filename in filenames:
            if filename.endswith(".c"):
                c_file_path = os.path.join(dirpath, filename)
                create_header_file(c_file_path)

        for dirname in dirnames:
            subdir_path = os.path.join(dirpath, dirname)
            create_c_and_h_files_for_empty_directory(subdir_path)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python generate_headers.py <root_directory>")
        sys.exit(1)

    root_directory = sys.argv[1]
    scan_directory(root_directory)
