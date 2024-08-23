import os
import filecmp
from pathlib import Path

salam_bin = "/mnt/c/Users/MAX/Salam/src/main"

def run_tests_in_directory(directory):
	output_dir = directory / "output"
	
	if "output" in directory.parts:
		return

	output_dir.mkdir(exist_ok=True)

	os.chdir(output_dir)

	parent_layout_file = directory / "layout.salam"
	if parent_layout_file.exists():
		os.system(f"{salam_bin} {parent_layout_file}")
	else:
		print(f"{parent_layout_file} does not exist. Skipping salam command.")

def compare_output_to_expected(directory):
	output_dir = directory / "output"
	
	comparison = filecmp.dircmp(directory, output_dir)

	if comparison.diff_files:
		print(f"Differences found in directory: {directory}")
		for file in comparison.diff_files:
			print(f" - {file} differs")
	else:
		print(f"No differences found in directory: {directory}")

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
