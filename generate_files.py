import os
import sys
import re

IGNORED_ROOT_HEADERS = {"base.h", "base_all.h", "main.h"}

def generate_guard_from_path(h_file_path, root):
    rel_path = os.path.relpath(h_file_path, root)
    parts = rel_path.split(os.sep)
    guard = "_".join(parts).upper().replace(".", "_") + "_"
    guard = "_" + guard
    return guard

def clean_and_regenerate_guard(h_file_path, root):
    if not os.path.exists(h_file_path):
        return

    with open(h_file_path, "r", encoding="utf-8") as file:
        lines = file.readlines()

    guard_define_line = None
    guard_ifndef_line = None
    endif_line = None
    inside_guard = False

    new_lines = []
    for i, line in enumerate(lines):
        if re.match(r"#\s*ifndef\s+_[A-Z0-9_]+", line):
            guard_ifndef_line = i
            inside_guard = True
            continue
        elif inside_guard and re.match(r"#\s*define\s+_[A-Z0-9_]+", line):
            guard_define_line = i
            continue
        elif inside_guard and re.match(r"#\s*endif\s*(//.*)?", line):
            endif_line = i
            inside_guard = False
            continue
        elif not inside_guard:
            new_lines.append(line)

    guard = generate_guard_from_path(h_file_path, root)
    content = f"#ifndef {guard}\n#define {guard}\n\n" + "".join(new_lines).strip() + f"\n\n#endif // {guard}\n"

    with open(h_file_path, "w", encoding="utf-8") as file:
        file.write(content)

    print(f"[REGENERATE] Header guard: {h_file_path}")

def create_header_file(c_file_path, root):
    base_name = os.path.splitext(os.path.basename(c_file_path))[0]
    h_file_path = os.path.join(os.path.dirname(c_file_path), base_name + ".h")

    if os.path.exists(h_file_path):
        clean_and_regenerate_guard(h_file_path, root)
        return

    guard = generate_guard_from_path(h_file_path, root)
    content = f"#ifndef {guard}\n#define {guard}\n\n// base\n#include <base.h>\n\n... {base_name}(...);\n\n#endif // {guard}\n"

    with open(h_file_path, "w", encoding="utf-8") as h_file:
        h_file.write(content)

    print(f"[CREATE] Header: {h_file_path}")

def create_c_and_h_files_for_empty_directory(directory, root):
    if not os.path.isdir(directory):
        print(f"[SKIP] Not a directory: {directory}")
        return
    if os.listdir(directory):
        print(f"[SKIP] Not empty: {directory}")
        return

    base_name = os.path.basename(os.path.abspath(directory))
    c_path = os.path.join(directory, base_name + ".c")
    h_path = os.path.join(directory, base_name + ".h")

    with open(c_path, "w", encoding="utf-8") as c_file:
        c_file.write(f'#include "{base_name}.h"\n\n... {base_name}(...)\n' + "{\n\n}\n")
    print(f"[CREATE] C File: {c_path}")

    guard = generate_guard_from_path(h_path, root)
    with open(h_path, "w", encoding="utf-8") as h_file:
        h_file.write(f"#ifndef {guard}\n#define {guard}\n\n\n\n#endif // {guard}\n")
    print(f"[CREATE] Header: {h_path}")

def scan_directory(root):
    for dirpath, dirnames, filenames in os.walk(root):
        for filename in filenames:
            if filename.endswith(".c"):
                c_file_path = os.path.join(dirpath, filename)
                create_header_file(c_file_path, root)

        for filename in filenames:
            if filename.endswith(".h"):
                h_file_path = os.path.join(dirpath, filename)

                if dirpath == root and filename in IGNORED_ROOT_HEADERS:
                    print(f"[SKIP] Ignored root header: {h_file_path}")
                    continue

                clean_and_regenerate_guard(h_file_path, root)

        for dirname in dirnames:
            subdir_path = os.path.join(dirpath, dirname)
            create_c_and_h_files_for_empty_directory(subdir_path, root)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python generate_headers.py <root_directory>")
        sys.exit(1)

    root_directory = os.path.abspath(sys.argv[1])
    scan_directory(root_directory)
