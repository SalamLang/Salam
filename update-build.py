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

# === File Collection ===
def collect_files_by_extension(src_dir, extension, exclude=None, rel_start=None):
    """Collect files with specific extension from src_dir, optionally exclude some, return sorted list."""
    exclude = exclude or set()
    files_collected = []
    for root, _, files in os.walk(src_dir):
        for f in files:
            if f.endswith(extension) and f not in exclude:
                full_path = os.path.join(root, f)
                rel_path = os.path.relpath(full_path, start=rel_start or os.getcwd())
                files_collected.append(rel_path.replace("\\", "/"))
    print(f"📂 Collected {len(files_collected)} '{extension}' files from '{src_dir}'")
    return sorted(files_collected)

def collect_c_files(src_dir):
    return collect_files_by_extension(src_dir, ".c")

def collect_h_files(src_dir):
    return collect_files_by_extension(src_dir, ".h", exclude=EXCLUDED_HEADERS, rel_start=SRC_DIR)

# === Formatters ===
def format_for_cmake(files):
    formatted = "\n".join(f"    {f}" for f in files)
    print(f"✍️ Formatted {len(files)} entries for CMake")
    return formatted

def format_for_make(files):
    formatted = "\n".join(f"\t{f} \\" for f in files)
    print(f"✍️ Formatted {len(files)} entries for Makefile")
    return formatted

def format_for_make_objects(files):
    formatted = "\n".join(f"\t{f} \\" for f in files)
    print(f"✍️ Formatted {len(files)} object file entries for Makefile")
    return formatted

# === Update file content ===
def update_makefile_content(content, new_file_list):
    new_block = f"{START_FILES_MARKER}\nSRCS := \\\n{new_file_list.rstrip(' \\')}\n{END_FILES_MARKER}"
    updated_content, count = re.subn(
        rf"{re.escape(START_FILES_MARKER)}.*?{re.escape(END_FILES_MARKER)}",
        new_block,
        content,
        flags=re.DOTALL
    )
    if count == 0:
        print(f"⚠️ Markers not found. Skipping update.")
        return None
    return updated_content

def update_makefile_objects(content, new_obj_list):
    new_block = f"{START_OBJECTS_MARKER}\nOBJS := \\\n{new_obj_list.rstrip(' \\')}\n{END_OBJECTS_MARKER}"
    updated_content, count = re.subn(
        rf"{re.escape(START_OBJECTS_MARKER)}.*?{re.escape(END_OBJECTS_MARKER)}",
        new_block,
        content,
        flags=re.DOTALL
    )
    if count == 0:
        print(f"⚠️ Object markers not found. Skipping objects update.")
        return None
    return updated_content

def update_generic_file_content(content, new_file_list):
    new_block = f"{START_FILES_MARKER}\n{new_file_list.rstrip(' \\')}\n    {END_FILES_MARKER}"
    updated_content, count = re.subn(
        rf"{re.escape(START_FILES_MARKER)}.*?{re.escape(END_FILES_MARKER)}",
        new_block,
        content,
        flags=re.DOTALL
    )
    if count == 0:
        print(f"⚠️ Markers not found. Skipping update.")
        return None
    return updated_content

def update_file(filepath, files, formatter):
    print(f"🔄 Updating '{filepath}'...")
    with open(filepath, "r", encoding="utf-8") as f:
        content = f.read()

    new_file_list = formatter(files)

    if filepath == MAKEFILE:
        updated_content = update_makefile_content(content, new_file_list)
    else:
        updated_content = update_generic_file_content(content, new_file_list)

    if updated_content is None:
        print(f"❌ Failed to update '{filepath}'.")
        return

    with open(filepath, "w", encoding="utf-8") as f:
        f.write(updated_content)

    print(f"✅ Updated '{filepath}' with {len(files)} files.")

def update_makefile_objects_section(filepath, o_files):
    print(f"🔄 Updating object files section in '{filepath}'...")
    with open(filepath, "r", encoding="utf-8") as f:
        content = f.read()

    new_obj_list = format_for_make_objects(o_files)
    updated_content = update_makefile_objects(content, new_obj_list)

    if updated_content is None:
        print(f"❌ Failed to update object files section in '{filepath}'.")
        return

    with open(filepath, "w", encoding="utf-8") as f:
        f.write(updated_content)

    print(f"✅ Updated '{filepath}' object files section with {len(o_files)} entries.")

# === Utility ===
def c_files_to_o_files(c_files, build_dir="build"):
    """Convert list of .c files to corresponding .o files in build_dir."""
    o_files = []
    for c_file in c_files:
        if c_file.startswith(SRC_DIR + "/"):
            rel_path = c_file[len(SRC_DIR) + 1:]
        else:
            rel_path = c_file

        o_file = os.path.splitext(rel_path)[0] + ".o"
        o_file_path = os.path.join(build_dir, o_file).replace("\\", "/")
        o_files.append(o_file_path)
    return sorted(o_files)

# === Write auxiliary files ===
def ensure_include_guards_in_headers(h_files, src_dir):
    for rel_path in h_files:
        full_path = os.path.join(src_dir, rel_path)
        with open(full_path, "r", encoding="utf-8") as f:
            lines = f.readlines()

        if lines and lines[0].strip().startswith("#ifndef"):
            continue

        macro = "_" + re.sub(r'[^a-zA-Z0-9]', '_', rel_path.upper()) + "_"
        guarded_content = [
            f"#ifndef {macro}\n",
            f"#define {macro}\n\n",
            *lines,
            f"\n#endif // {macro}\n"
        ]

        with open(full_path, "w", encoding="utf-8") as f:
            f.writelines(guarded_content)

        print(f"🛡️  Added include guard to '{rel_path}'")

def write_c_file_list(files, output_path):
    with open(output_path, "w", encoding="utf-8") as f:
        f.write("\n".join(files) + "\n")
    print(f"📄 Generated '{output_path}' with {len(files)} entries.")

def generate_base_all_header(h_files, output_path):
    include_lines = "\n".join(f'#include <{f}>' for f in h_files)
    content = f"""#ifndef _BASE_ALL_H_
#define _BASE_ALL_H_
// Auto-generated header including all .h files

{include_lines}

#endif // _BASE_ALL_H_
"""
    with open(output_path, "w", encoding="utf-8") as f:
        f.write(content)
    print(f"🧩 Generated '{output_path}' with {len(h_files)} includes.")

# === Main ===
if __name__ == "__main__":
    print("🚀 Starting file generation process...\n")

    c_files = collect_c_files(SRC_DIR)
    update_file(CMAKE_FILE, c_files, format_for_cmake)
    update_file(MAKEFILE, c_files, format_for_make)
    write_c_file_list(c_files, C_FILES_TXT)

    o_files = c_files_to_o_files(c_files)
    update_makefile_objects_section(MAKEFILE, o_files)

    h_files = collect_h_files(SRC_DIR)
    ensure_include_guards_in_headers(h_files, SRC_DIR)
    generate_base_all_header(h_files, BASE_ALL_HEADER)

    print("\n🎉 All done!")
