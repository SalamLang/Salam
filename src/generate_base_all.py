import os

base_dir = os.getcwd()
output_file = os.path.join(base_dir, "base_all.h")

header_files = []

for dirpath, _, filenames in os.walk(base_dir):
    for filename in filenames:
        if filename.endswith(".h"):
            full_path = os.path.join(dirpath, filename)
            rel_path = os.path.relpath(full_path, base_dir)
            header_files.append(rel_path)

with open(output_file, "w", encoding="utf-8") as f:
    f.write("// Auto-generated header including all .h files\n\n")
    for header in sorted(header_files):
        f.write(f'#include "{header}"\n')

print(f"Generated {output_file} with {len(header_files)} includes.")
