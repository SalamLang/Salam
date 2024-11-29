import filecmp
import os
from pathlib import Path

# salam_bin = "/mnt/c/Users/MAX/Salam/src/main"
salam_bin = "../salam"

total_tests = 0
passed_tests = 0
failed_tests = 0
warnings = 0

COLOR_RESET = "\033[0m"
COLOR_GREEN = "\033[92m"
COLOR_RED = "\033[91m"
COLOR_BLUE = "\033[94m"


def run_tests_in_directory(directory):
    output_dir = directory / "output"

    if "output" in directory.parts:
        return

    output_dir.mkdir(exist_ok=True)

    os.chdir(output_dir)

    parent_layout_file = directory / "layout.salam"
    if parent_layout_file.exists():
        os.system(f"{salam_bin} {parent_layout_file} > /dev/null 2>&1")
    else:
        print(
            f"{COLOR_RED}{parent_layout_file} does not exist. Skipping salam command.{COLOR_RESET}"
        )


def compare_output_to_expected(directory):
    global total_tests, passed_tests, failed_tests, warnings

    output_dir = directory / "output"

    comparison = filecmp.dircmp(directory, output_dir)

    test_failed = False

    if comparison.diff_files:
        test_failed = True
        print(f"{COLOR_RED}Differences found in directory: {directory}{COLOR_RESET}")
        for file in comparison.diff_files:
            print(f" - {COLOR_RED}{file} differs{COLOR_RESET}")
    elif comparison.right_only:
        warnings += 1
        print(
            f"{COLOR_BLUE}Warning: Extra files found in the output directory of {directory}:{COLOR_RESET}"
        )
        for file in comparison.right_only:
            print(f" - {COLOR_BLUE}{file}{COLOR_RESET}")
    else:
        print(f"{COLOR_GREEN}Test case passed: {directory}{COLOR_RESET}")

    if test_failed:
        failed_tests += 1
    else:
        passed_tests += 1

    total_tests += 1


def process_directory(directory):
    salam_files = list(directory.glob("*.salam"))
    if not salam_files:
        return

    run_tests_in_directory(directory)
    compare_output_to_expected(directory)


def iterate_directories(base_dir):
    for root, dirs, files in os.walk(base_dir):
        dirs[:] = [d for d in dirs if d != "output"]

        for subdir in dirs:
            full_path = Path(root) / subdir
            process_directory(full_path)


if __name__ == "__main__":
    current_directory = Path(os.getcwd())
    iterate_directories(current_directory)

    print("\n--- Test Summary ---")
    print(f"{COLOR_GREEN}Total test cases: {total_tests}{COLOR_RESET}")
    print(f"{COLOR_GREEN}Passed test cases: {passed_tests}{COLOR_RESET}")
    print(f"{COLOR_RED}Failed test cases: {failed_tests}{COLOR_RESET}")
    print(f"{COLOR_BLUE}Warnings: {warnings}{COLOR_RESET}")
