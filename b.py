import subprocess
import sys
import os
import shutil
from pathlib import Path
from concurrent.futures import ProcessPoolExecutor
import argparse
import logging
import hashlib
from typing import List, Tuple, Optional

logging.basicConfig(level=logging.INFO, format='[%(levelname)s] %(message)s')

# ----------- Configuration -----------
JSON_FILES_TO_BEAUTIFY = ["tokens.json", "ast.json"]
COMPILE_TIMEOUT = 60  # seconds
LINK_TIMEOUT = 60

JQ_PATH: Optional[str] = shutil.which("jq")

# ----------- Helper Functions -----------
def run_command(cmd: List[str], capture_output: bool = False, timeout: Optional[int] = None) -> Optional[str]:
    """Run a system command safely, returning stdout if capture_output is True."""
    try:
        if capture_output:
            result = subprocess.run(cmd, check=True, capture_output=True, text=True, timeout=timeout)
            return result.stdout
        else:
            subprocess.run(cmd, check=True, timeout=timeout)
    except subprocess.CalledProcessError as e:
        logging.error(f"Command failed: {' '.join(cmd)}")
        if e.stdout:
            logging.error(e.stdout)
        if e.stderr:
            logging.error(e.stderr)
        sys.exit(1)
    except subprocess.TimeoutExpired:
        logging.error(f"Command timed out: {' '.join(cmd)}")
        sys.exit(1)
    return None

def file_hash(filepath: Path) -> str:
    """Return SHA-256 hash of the file contents."""
    hasher = hashlib.sha256()
    with filepath.open('rb') as f:
        while chunk := f.read(8192):
            hasher.update(chunk)
    return hasher.hexdigest()

def compile_c_file(args: Tuple[str, str, str]) -> str:
    filename, compiler, cflags = args
    src_path = Path(filename)
    obj_file = src_path.with_suffix('.o')
    hash_file = obj_file.with_suffix('.o.hash')

    src_hash = file_hash(src_path)

    if obj_file.exists() and hash_file.exists():
        cached_hash = hash_file.read_text().strip()
        if cached_hash == src_hash:
            logging.info(f"Skipping compilation (cached): {filename}")
            return str(obj_file)

    cmd = [compiler, "-c"] + cflags.split() + [filename, "-o", str(obj_file)]
    logging.info(f"Compiling {filename} ...")
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, timeout=COMPILE_TIMEOUT)
    if result.returncode != 0:
        logging.error(f"Compilation failed for {filename}:\n{result.stderr}")
        sys.exit(1)

    # Save hash to detect changes later
    hash_file.write_text(src_hash)
    return str(obj_file)

def link_objects(compiler: str, output: str, obj_files: List[str], ldflags: str) -> None:
    """Link object files into final executable."""
    cmd = [compiler, "-o", output] + obj_files + ldflags.split()
    logging.info(f"Linking {output} ...")
    run_command(cmd, timeout=LINK_TIMEOUT)

def run_program(executable: str, args: List[str]) -> None:
    """Run the compiled executable with provided arguments."""
    exec_path = Path(f"./{executable}")
    if not exec_path.exists():
        logging.error(f"Executable {executable} not found. Cannot run program.")
        sys.exit(1)
    logging.info(f"Running program: {executable} {' '.join(args)}")
    try:
        subprocess.run([str(exec_path)] + args, check=True)
        logging.info("Program executed successfully.")
    except subprocess.CalledProcessError:
        logging.error("Program execution failed!")
        sys.exit(1)

def beautify_json_if_valid(filepath: str) -> None:
    """Beautify JSON file if valid, using jq tool."""
    path = Path(filepath)
    if not path.exists():
        logging.warning(f"{filepath} not found. Skipping beautification.")
        return

    if JQ_PATH is None:
        logging.warning("'jq' not installed. Skipping beautification.")
        return

    logging.info(f"Checking if {filepath} is valid JSON...")
    result = subprocess.run([JQ_PATH, "empty", filepath], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if result.returncode == 0:
        pretty_file = path.with_suffix(".pretty")
        with pretty_file.open("w") as pf:
            subprocess.run([JQ_PATH, ".", filepath], stdout=pf, check=True)
        shutil.move(str(pretty_file), str(path))
        logging.info(f"Beautification of {filepath} complete.")
    else:
        logging.warning(f"{filepath} contains invalid JSON. Skipping.")

def clean_build(c_files: List[str]) -> None:
    """Remove object and hash files for a clean rebuild."""
    for filename in c_files:
        obj_file = Path(filename).with_suffix('.o')
        hash_file = obj_file.with_suffix('.o.hash')
        for f in [obj_file, hash_file]:
            if f.exists():
                logging.info(f"Removing {f}")
                f.unlink()

# ----------- Main -----------
def main() -> None:
    """Parse arguments and run build pipeline."""
    parser = argparse.ArgumentParser(
        description="Parallel C build script",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument("--compiler", default="gcc", choices=["gcc", "tcc"], help="Compiler to use")
    parser.add_argument("--output", default="s", help="Output executable name")
    parser.add_argument("--files", default="c_files.txt", help="File containing list of C source files")
    parser.add_argument("--jobs", type=int, default=os.cpu_count(), help="Number of parallel compile jobs")
    parser.add_argument("--beautify-json", action="store_true", help="Beautify tokens.json and ast.json if valid")
    parser.add_argument("--run-args", nargs="*", default=["input.salam"], help="Arguments to pass when running the program")
    parser.add_argument("--clean", action="store_true", help="Clean object and hash files before building")
    args = parser.parse_args()

    try:
        source_list_path = Path(args.files)
        if not source_list_path.exists():
            logging.error(f"Source file list '{args.files}' not found.")
            sys.exit(1)

        with source_list_path.open() as f:
            c_files = [line.strip() for line in f if line.strip()]

        if not c_files:
            logging.error("No source files found to compile.")
            sys.exit(1)

        if args.clean:
            clean_build(c_files)

        cflags_common = "-Wno-unused-parameter -g -Wall -Wextra -pedantic"
        include_dirs = "-I. -Isrc"
        cflags = f"{cflags_common} {include_dirs}"
        ldflags = ""

        if args.compiler == "tcc":
            logging.info("tcc selected (non-parallel)...")
            obj_files = []
            for c_file in c_files:
                obj_file = compile_c_file((c_file, args.compiler, cflags))
                obj_files.append(obj_file)
        else:
            logging.info(f"{args.compiler} selected (parallel compile, max jobs={args.jobs})...")
            with ProcessPoolExecutor(max_workers=args.jobs) as executor:
                obj_files = list(executor.map(compile_c_file, [(f, args.compiler, cflags) for f in c_files]))

        link_objects(args.compiler, args.output, obj_files, ldflags)
        run_program(args.output, args.run_args)

        if args.beautify_json:
            for json_file in JSON_FILES_TO_BEAUTIFY:
                beautify_json_if_valid(json_file)

    except KeyboardInterrupt:
        logging.warning("Build interrupted by user.")
        sys.exit(130)

if __name__ == "__main__":
    main()
