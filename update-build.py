import os
import re

SRC_DIR = "src"
CMAKE_FILE = "CMakeLists.txt"
MAKEFILE = "Makefile"
C_FILES_TXT = "c_files.txt"
BASE_ALL_HEADER = os.path.join(SRC_DIR, "base_all.h")

START_FILES_MARKER = "# ---------- START FILES ----------"
END_FILES_MARKER = "# ---------- END FILES ----------"

START_OBJECTS_MARKER = "# ---------- START OBJECT FILES ----------"
END_OBJECTS_MARKER = "# ---------- END OBJECT FILES ----------"

EXCLUDED_HEADERS = {"main.h", "base.h", "base_all.h"}


# === Log ===
def log_info(msg):
    print(f"ℹ️ {msg}")


def log_warn(msg):
    print(f"⚠️ {msg}")


def log_error(msg):
    print(f"❌ {msg}")


def log_success(msg):
    print(f"✅ {msg}")


# === Utility ===
def generate_guard_from_path(h_file_path, root):
    rel_path = os.path.relpath(h_file_path, root)
    parts = rel_path.split(os.sep)
    guard = "_" + "_".join(parts).upper().replace(".", "_") + "_"
    return guard


# === File Collection ===
def collect_files_by_extension(src_dir, extension, exclude=None, rel_start=None):
    exclude = exclude or set()
    collected = []
    for root, _, files in os.walk(src_dir):
        for f in files:
            if f.endswith(extension) and f not in exclude:
                full_path = os.path.join(root, f)
                rel_path = os.path.relpath(full_path, start=rel_start or os.getcwd())
                collected.append(rel_path.replace("\\", "/"))
    log_info(f"Collected {len(collected)} '{extension}' files from '{src_dir}'")
    return sorted(collected)


def collect_c_files(src_dir):
    return collect_files_by_extension(src_dir, ".c")


def collect_h_files(src_dir):
    return collect_files_by_extension(
        src_dir, ".h", exclude=EXCLUDED_HEADERS, rel_start=SRC_DIR
    )


# === Formatters ===
def format_for_cmake(files):
    log_info(f"Formatting {len(files)} entries for CMake")
    return "\n".join(f"    {f}" for f in files)


def format_for_make(files):
    log_info(f"Formatting {len(files)} entries for Makefile")
    return "\n".join(f"\t{f} \\" for f in files)


def format_for_make_objects(files):
    log_info(f"Formatting {len(files)} object file entries for Makefile")
    return "\n".join(f"\t{f} \\" for f in files)


# === Update file content ===
def generate_base_all_header(h_files, output_path):
    includes = "\n".join(f"#include <{f}>" for f in h_files)
    content = f"""#ifndef _BASE_ALL_H_
#define _BASE_ALL_H_
// Auto-generated header including all .h files

{includes}

#endif // _BASE_ALL_H_
"""
    with open(output_path, "w", encoding="utf-8") as f:
        f.write(content)
    log_success(f"Generated '{output_path}' with {len(h_files)} includes.")


def update_marked_block(content, marker_start, marker_end, body):
    new_block = f"{marker_start}\n{body.rstrip(' \\')}\n{marker_end}"
    updated, count = re.subn(
        rf"{re.escape(marker_start)}.*?{re.escape(marker_end)}",
        new_block,
        content,
        flags=re.DOTALL,
    )
    if count == 0:
        log_warn(f"Markers '{marker_start}' not found.")
        return None
    return updated


def update_file(filepath, files, formatter):
    log_info(f"Updating '{filepath}'...")
    with open(filepath, encoding="utf-8") as f:
        content = f.read()

    new_list = formatter(files)

    if filepath == MAKEFILE:
        updated = update_marked_block(
            content, START_FILES_MARKER, END_FILES_MARKER, f"SRCS := \\\n{new_list}"
        )
    else:
        updated = update_marked_block(
            content, START_FILES_MARKER, END_FILES_MARKER, new_list
        )

    if updated is None:
        log_error(f"Failed to update '{filepath}'.")
        return

    with open(filepath, "w", encoding="utf-8") as f:
        f.write(updated)

    log_success(f"Updated '{filepath}' with {len(files)} files.")


def update_makefile_objects_section(filepath, o_files):
    log_info(f"Updating object files section in '{filepath}'...")
    with open(filepath, encoding="utf-8") as f:
        content = f.read()

    new_obj_list = format_for_make_objects(o_files)
    updated = update_marked_block(
        content, START_OBJECTS_MARKER, END_OBJECTS_MARKER, f"OBJS := \\\n{new_obj_list}"
    )

    if updated is None:
        log_error(f"Failed to update object files section in '{filepath}'.")
        return

    with open(filepath, "w", encoding="utf-8") as f:
        f.write(updated)

    log_success(
        f"Updated object files section in '{filepath}' with {len(o_files)} entries."
    )


# === Header Guarding ===
def ensure_include_guards_in_headers(h_files, src_dir):
    for rel_path in h_files:
        full_path = os.path.join(src_dir, rel_path)
        with open(full_path, encoding="utf-8") as f:
            lines = f.readlines()

        if lines and lines[0].strip().startswith("#ifndef"):
            continue

        macro = generate_guard_from_path(full_path, src_dir)
        guarded = [
            f"#ifndef {macro}\n",
            f"#define {macro}\n\n",
            *lines,
            f"\n#endif // {macro}\n",
        ]

        with open(full_path, "w", encoding="utf-8") as f:
            f.writelines(guarded)

        log_success(f"Added include guard to '{rel_path}'")


# === Object File Conversion ===
def c_files_to_o_files(c_files, build_dir="build"):
    o_files = []
    for c_file in c_files:
        rel_path = (
            c_file[len(SRC_DIR) + 1 :] if c_file.startswith(SRC_DIR + "/") else c_file
        )
        o_path = os.path.join(build_dir, os.path.splitext(rel_path)[0] + ".o").replace(
            "\\", "/"
        )
        o_files.append(o_path)
    return sorted(o_files)


# === File Writer ===
def write_c_file_list(files, output_path):
    with open(output_path, "w", encoding="utf-8") as f:
        f.write("\n".join(files) + "\n")
    log_success(f"Generated '{output_path}' with {len(files)} entries.")


# === Main ===
if __name__ == "__main__":
    log_info("Starting file generation process...\n")

    c_files = collect_c_files(SRC_DIR)
    update_file(CMAKE_FILE, c_files, format_for_cmake)
    update_file(MAKEFILE, c_files, format_for_make)
    write_c_file_list(c_files, C_FILES_TXT)

    o_files = c_files_to_o_files(c_files)
    update_makefile_objects_section(MAKEFILE, o_files)

    h_files = collect_h_files(SRC_DIR)
    ensure_include_guards_in_headers(h_files, SRC_DIR)
    generate_base_all_header(h_files, BASE_ALL_HEADER)

    log_success("All done!")
