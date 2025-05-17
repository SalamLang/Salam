import os
import re

SRC_DIR = "src"
CMAKE_FILE = "CMakeLists.txt"

def collect_c_files(src_dir):
    c_files = []
    for root, _, files in os.walk(src_dir):
        for f in files:
            if f.endswith(".c"):
                full_path = os.path.join(root, f)
                rel_path = os.path.relpath(full_path, start=os.getcwd())
                c_files.append(rel_path.replace("\\", "/"))
    return sorted(c_files)

def update_cmake_file(cmake_file, files):
    with open(cmake_file, "r") as f:
        content = f.read()

    start_marker = "# ---------- START FILES ----------"
    end_marker = "# ---------- END FILES ----------"

    new_file_list = "\n".join(f"    {f}" for f in files)
    new_block = f"{start_marker}\n{new_file_list}\n    {end_marker}"

    updated_content = re.sub(
        f"{start_marker}.*?{end_marker}",
        new_block,
        content,
        flags=re.DOTALL
    )

    with open(cmake_file, "w") as f:
        f.write(updated_content)

    print(f"Updated {cmake_file} with {len(files)} source files.")

if __name__ == "__main__":
    c_files = collect_c_files(SRC_DIR)
    update_cmake_file(CMAKE_FILE, c_files)
