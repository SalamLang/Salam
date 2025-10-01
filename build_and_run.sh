#!/bin/bash

cd build || exit 1

make
if [ $? -ne 0 ]; then
	echo "Build failed!"
	exit 1
fi

echo "Running program with Valgrind..."
valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes ./salam src/input.salam

EXIT_CODE=$?
if [ $EXIT_CODE -ne 0 ]; then
	echo "Program failed. Skipping beautification."
	exit $EXIT_CODE
fi

beautify_json_if_valid() {
	local file="$1"
	if [ -f "$file" ]; then
		if command -v jq >/dev/null 2>&1; then
			if jq empty "$file" >/dev/null 2>&1; then
				jq . "$file" >"${file}.pretty" && mv "${file}.pretty" "$file"
				echo "Beautified $file"
			else
				echo "$file is invalid JSON. Skipping."
			fi
		fi
	fi
}

beautify_json_if_valid src/tokens.json
beautify_json_if_valid src/ast.json
