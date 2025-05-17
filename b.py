import subprocess
import os
import sys
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor

# ----------- Configuration -----------
COMPILER = "gcc"  # "tcc" also supported (basic, non-parallel)
SALAM_OUTPUT = "s"
C_FILES_TXT = "c_files.txt"

CFLAGS_COMMON = "-Wno-unused-parameter -g -Wall -Wextra -pedantic"
INCLUDE_DIRS = "-I. -Isrc"
CFLAGS = f"{CFLAGS_COMMON} {INCLUDE_DIRS}"
LDFLAGS = ""

# ----------- Helper Functions -----------
def run_command(cmd, capture_output=False):
    try:
        if capture_output:
            return subprocess.run(cmd, check=True, shell=True, capture_output=True, text=True)
        else:
            subprocess.run(cmd, check=True, shell=True)
    except subprocess.CalledProcessError as e:
        print(f"Command failed: {cmd}")
        if e.stdout:
            print(e.stdout)
        if e.stderr:
            print(e.stderr)
        sys.exit(1)

def compile_c_file(filename):
    obj_file = f"{Path(filename).stem}.o"
    cmd = f"{COMPILER} -c {CFLAGS} {filename} -o {obj_file}"
    print(f"Compiling {filename} ...")
    result = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if result.returncode != 0:
        print(f"Compilation failed for {filename}:\n{result.stderr}")
        sys.exit(1)
    return obj_file

def link_objects(obj_files):
    obj_str = " ".join(obj_files)
    print("Linking...")
    cmd = f"{COMPILER} -o {SALAM_OUTPUT} {obj_str} {LDFLAGS}"
    run_command(cmd)

def run_program():
    print("Running program...")
    try:
        subprocess.run(f"./{SALAM_OUTPUT} input.salam", shell=True, check=True)
        print("Program executed successfully.")
    except subprocess.CalledProcessError:
        print("Program execution failed!")
        sys.exit(1)

def beautify_json_if_valid(filepath):
    if not Path(filepath).exists():
        print(f"{filepath} not found. Skipping beautification.")
        return

    if subprocess.run("command -v jq", shell=True).returncode != 0:
        print("Warning: 'jq' not installed. Skipping beautification.")
        return

    print(f"Checking if {filepath} is valid JSON...")
    if subprocess.run(f"jq empty {filepath}", shell=True).returncode == 0:
        pretty_file = Path(filepath).with_suffix(".pretty")
        subprocess.run(f"jq . {filepath} > {pretty_file}", shell=True)
        os.replace(pretty_file, filepath)
        print(f"Beautification of {filepath} complete.")
    else:
        print(f"{filepath} contains invalid JSON. Skipping.")

# ----------- Main Script -----------
def main():
    if not Path(C_FILES_TXT).exists():
        print(f"{C_FILES_TXT} not found.")
        sys.exit(1)

    with open(C_FILES_TXT) as f:
        c_files = [line.strip() for line in f if line.strip()]

    if COMPILER == "tcc":
        print("tcc selected (non-parallel)...")
        obj_files = []
        for c_file in c_files:
            obj_file = compile_c_file(c_file)
            obj_files.append(obj_file)
    elif COMPILER == "gcc":
        print("gcc selected (parallel compile)...")
        with ThreadPoolExecutor() as executor:
            obj_files = list(executor.map(compile_c_file, c_files))
    else:
        print(f"Unknown compiler: {COMPILER}")
        sys.exit(1)

    link_objects(obj_files)
    run_program()

    beautify_json_if_valid("tokens.json")
    beautify_json_if_valid("ast.json")

if __name__ == "__main__":
    main()
