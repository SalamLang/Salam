import os
import re

SRC_DIR = "src"
CMAKE_FILE = "CMakeLists.txt"
MAKEFILE = "Makefile"
C_FILES_TXT = "c_files.txt"
BASE_ALL_HEADER = os.path.join(SRC_DIR, "base_all.h")

START_MARKER = "# ---------- START FILES ----------"
END_MARKER = "# ---------- END FILES ----------"

EXCLUDED_HEADERS = {"main.h", "base.h", "base_all.h"}

def collect_c_files(src_dir):
    c_files = []
    for root, _, files in os.walk(src_dir):
        for f in files:
            if f.endswith(".c"):
                full_path = os.path.join(root, f)
                rel_path = os.path.relpath(full_path, start=os.getcwd())
                c_files.append(rel_path.replace("\\", "/"))
    return sorted(c_files)

def collect_h_files(src_dir):
    h_files = []
    for root, _, files in os.walk(src_dir):
        for f in files:
            if f.endswith(".h") and f not in EXCLUDED_HEADERS:
                full_path = os.path.join(root, f)
                rel_path = os.path.relpath(full_path, start=SRC_DIR)
                h_files.append(rel_path.replace("\\", "/"))
    return sorted(h_files)

def format_for_cmake(files):
    return "\n".join(f"    {f}" for f in files)

def format_for_make(files):
    return "\n".join(f"\t{f} \\" for f in files)

def update_file(filepath, files, formatter):
    with open(filepath, "r") as f:
        content = f.read()

    new_file_list = formatter(files)
    new_block = f"{START_MARKER}\n{new_file_list.rstrip(' \\')}\n    {END_MARKER}"

    updated_content, count = re.subn(
        f"{re.escape(START_MARKER)}.*?{re.escape(END_MARKER)}",
        new_block,
        content,
        flags=re.DOTALL
    )

    if count == 0:
        print(f"Warning: Markers not found in {filepath}. Skipping update.")
        return

    with open(filepath, "w") as f:
        f.write(updated_content)

    print(f"Updated {filepath} with {len(files)} source files.")

def write_c_file_list(files, output_path):
    with open(output_path, "w") as f:
        f.write("\n".join(files) + "\n")
    print(f"Generated {output_path} with {len(files)} entries.")

def generate_base_all_header(h_files, output_path):
    include_lines = "\n".join(f'#include <{f}>' for f in h_files)
    content = f"""#ifndef _BASE_ALL_H_
#define _BASE_ALL_H_
// Auto-generated header including all .h files

{include_lines}

#endif // _BASE_ALL_H_
"""
    with open(output_path, "w") as f:
        f.write(content)

    print(f"Generated {output_path} with {len(h_files)} includes.")

if __name__ == "__main__":
    c_files = collect_c_files(SRC_DIR)
    update_file(CMAKE_FILE, c_files, format_for_cmake)
    update_file(MAKEFILE, c_files, format_for_make)
    write_c_file_list(c_files, C_FILES_TXT)

    h_files = collect_h_files(SRC_DIR)
    generate_base_all_header(h_files, BASE_ALL_HEADER)
