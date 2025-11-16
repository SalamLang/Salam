import argparse
import hashlib
import json
import logging
import os
import platform
import subprocess
import sys
from concurrent.futures import ProcessPoolExecutor
from pathlib import Path
from typing import List, Optional, Tuple

# ----------- Constants & Logging -----------
JSON_FILES_TO_BEAUTIFY = ["tokens.json", "ast.json"]
COMPILE_TIMEOUT = 60
LINK_TIMEOUT = 60

logger = logging.getLogger()
logging.basicConfig(level=logging.INFO, format="[%(levelname)s] %(message)s")


# ----------- Utility Functions -----------
def run_command(
    cmd: List[str], capture_output: bool = False, timeout: Optional[int] = None
) -> Optional[str]:
    try:
        if capture_output:
            result = subprocess.run(
                cmd, check=True, capture_output=True, text=True, timeout=timeout
            )
            return result.stdout
        subprocess.run(cmd, check=True, timeout=timeout)
        return None
    except subprocess.CalledProcessError as e:
        logger.error(f"Command failed: {' '.join(cmd)}")
        if e.stdout:
            logger.error(e.stdout)
        if e.stderr:
            logger.error(e.stderr)
        sys.exit(1)
    except subprocess.TimeoutExpired:
        logger.error(f"Command timed out: {''.join(cmd)}")
        sys.exit(1)


def file_hash(filepath: Path) -> str:
    hasher = hashlib.sha256()
    with filepath.open("rb") as f:
        while chunk := f.read(8192):
            hasher.update(chunk)
    return hasher.hexdigest()


def compile_c_file(args: Tuple[str, str, str]) -> str:
    filename, compiler, cflags = args
    src_path = Path(filename)
    obj_file = src_path.with_suffix(".o")
    hash_file = obj_file.with_suffix(".o.hash")

    src_hash = file_hash(src_path)

    if obj_file.exists() and hash_file.exists():
        cached_hash = hash_file.read_text().strip()
        if cached_hash == src_hash:
            logger.debug(f"Skipping compilation (cached): {filename}")
            return str(obj_file)

    cmd = [compiler, "-c"] + cflags.split() + [filename, "-o", str(obj_file)]
    logger.info(f"Compiling {filename} ...")
    result = subprocess.run(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        timeout=COMPILE_TIMEOUT,
    )
    if result.returncode != 0:
        logger.error(f"Compilation failed for {filename}:\n{result.stderr}")
        sys.exit(1)

    hash_file.write_text(src_hash)
    return str(obj_file)


def link_objects(
    compiler: str, output: str, obj_files: List[str], ldflags: str
) -> None:
    cmd = [compiler, "-o", output] + obj_files + ldflags.split()
    logger.info(f"Linking {output} ...")
    run_command(cmd, timeout=LINK_TIMEOUT)


def run_program(executable: str, args: List[str]) -> bool:
    if sys.platform.startswith("win") and not executable.lower().endswith(".exe"):
        executable += ".exe"

    exec_path = Path(executable)
    if not exec_path.exists():
        logger.error(f"Executable {exec_path} not found. Cannot run program.")
        return False

    if not sys.platform.startswith("win"):
        executable = "./" + executable
    logger.info(f"Running program: {executable} {' '.join(args)}")

    try:
        subprocess.run([executable, *args], check=True)
        logger.info("Program executed successfully.")
        return True
    except subprocess.CalledProcessError as e:
        logger.error(f"Program execution failed: {e}")
        return False


def beautify_json_if_valid(filepath: str) -> None:
    path = Path(filepath)
    if not path.exists():
        logger.warning(f"{filepath} not found. Skipping beautification.")
        return

    try:
        with path.open("r", encoding="utf-8") as f:
            data = json.load(f)

        with path.open("w", encoding="utf-8") as f:
            json.dump(data, f, indent=2, ensure_ascii=False)

        logger.info(f"Beautified {filepath} successfully.")
    except json.JSONDecodeError as e:
        logger.warning(f"{filepath} contains invalid JSON. Skipping. ({e})")
    except Exception as e:
        logger.error(f"Unexpected error while beautifying {filepath}: {e}")


def clean_build(c_files: List[str]) -> None:
    for filename in c_files:
        obj_file = Path(filename).with_suffix(".o")
        hash_file = obj_file.with_suffix(".o.hash")
        for f in [obj_file, hash_file]:
            if f.exists():
                logger.info(f"Removing {f}")
                f.unlink()


# ----------- Main Entry Point -----------
def main() -> None:
    parser = argparse.ArgumentParser(description="Parallel C build script")
    parser.add_argument(
        "--compiler", default="gcc", help="Compiler to use (e.g., gcc, clang, tcc)"
    )
    parser.add_argument("--output", default="s", help="Output executable name")
    parser.add_argument(
        "--files", default="c_files.txt", help="Text file containing list of .c files"
    )
    parser.add_argument(
        "--jobs",
        type=int,
        default=(os.cpu_count() or 1) * 3,  # fix: handle None properly
        help="Number of parallel compile jobs",
    )
    parser.add_argument(
        "--clean", action="store_true", help="Clean object/hash files before compiling"
    )
    parser.add_argument(
        "--run-args",
        nargs="*",
        default=["input.salam"],
        help="Arguments for running the executable",
    )
    parser.add_argument(
        "--no-run", action="store_true", help="Do not run the final executable"
    )
    parser.add_argument(
        "--beautify-json",
        action="store_true",
        default=True,
        help="Beautify JSON output files if valid",
    )
    parser.add_argument(
        "--cflags",
        default=os.getenv(
            "CFLAGS", "-Wno-unused-parameter -g -Wall -Wextra -pedantic -I. -Isrc"
        ),
        help="Compiler flags",
    )
    parser.add_argument(
        "--ldflags", default=os.getenv("LDFLAGS", ""), help="Linker flags"
    )
    parser.add_argument(
        "--verbose", action="store_true", help="Enable verbose debug logging"
    )
    args = parser.parse_args()

    if args.verbose:
        logger.setLevel(logging.DEBUG)

    try:
        source_list_path = Path(args.files)
        if not source_list_path.exists():
            logger.error(f"Source file list '{args.files}' not found.")
            sys.exit(1)

        with source_list_path.open() as f:
            c_files = [line.strip() for line in f if line.strip()]

        if not c_files:
            logger.error("No source files found.")
            sys.exit(1)

        if args.clean:
            clean_build(c_files)

        if platform.system() == "Windows" and not args.output.endswith(".exe"):
            args.output += ".exe"

        if args.compiler == "tcc":
            logger.info("Using TCC (non-parallel)...")
            obj_files = [
                compile_c_file((f, args.compiler, args.cflags)) for f in c_files
            ]
        else:
            logger.info(
                f"{args.compiler} selected (parallel compile, max jobs={args.jobs})"
            )
            with ProcessPoolExecutor(max_workers=args.jobs) as executor:
                obj_files = list(
                    executor.map(
                        compile_c_file,
                        [(f, args.compiler, args.cflags) for f in c_files],
                    )
                )

        link_objects(args.compiler, args.output, obj_files, args.ldflags)

        program_succeeded = True
        if not args.no_run:
            program_succeeded = run_program(args.output, args.run_args)

        if args.beautify_json:
            for json_file in JSON_FILES_TO_BEAUTIFY:
                beautify_json_if_valid(json_file)

        if not program_succeeded:
            sys.exit(1)

    except KeyboardInterrupt:
        logger.warning("Build interrupted by user.")
        sys.exit(130)


if __name__ == "__main__":
    main()
