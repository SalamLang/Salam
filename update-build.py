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

# === Update file content ===
def update_makefile_content(content, new_file_list):
    pattern = rf"(.*\n)?{re.escape(START_MARKER)}.*?{re.escape(END_MARKER)}"
    match = re.search(pattern, content, flags=re.DOTALL)
    if not match:
        print(f"⚠️  Markers not found in Makefile. Skipping update.")
        return None
    prev_line = match.group(1) or ""
    new_block = f"{prev_line}{START_MARKER}\n{new_file_list.rstrip(' \\')}\n    {END_MARKER}"
    return content[:match.start()] + new_block + content[match.end():]

def update_generic_file_content(content, new_file_list):
    new_block = f"{START_MARKER}\n{new_file_list.rstrip(' \\')}\n    {END_MARKER}"
    updated_content, count = re.subn(
        rf"{re.escape(START_MARKER)}.*?{re.escape(END_MARKER)}",
        new_block,
        content,
        flags=re.DOTALL
    )
    if count == 0:
        print(f"⚠️  Markers not found. Skipping update.")
        return None
    return updated_content

def update_file(filepath, files, formatter):
    print(f"🔄 Updating '{filepath}'...")
    with open(filepath, "r") as f:
        content = f.read()

    new_file_list = formatter(files)

    if filepath == MAKEFILE:
        updated_content = update_makefile_content(content, new_file_list)
    else:
        updated_content = update_generic_file_content(content, new_file_list)

    if updated_content is None:
        print(f"❌ Failed to update '{filepath}'.")
        return

    with open(filepath, "w") as f:
        f.write(updated_content)

    print(f"✅ Updated '{filepath}' with {len(files)} files.")

# === Write auxiliary files ===
def write_c_file_list(files, output_path):
    with open(output_path, "w") as f:
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
    with open(output_path, "w") as f:
        f.write(content)
    print(f"🧩 Generated '{output_path}' with {len(h_files)} includes.")

# === Main ===
if __name__ == "__main__":
    print("🚀 Starting file generation process...\n")

    c_files = collect_c_files(SRC_DIR)
    update_file(CMAKE_FILE, c_files, format_for_cmake)
    update_file(MAKEFILE, c_files, format_for_make)
    write_c_file_list(c_files, C_FILES_TXT)

    h_files = collect_h_files(SRC_DIR)
    generate_base_all_header(h_files, BASE_ALL_HEADER)

    print("\n🎉 All done!")
