import os
import sys

sys.path.append(
    os.path.abspath(os.path.join(os.path.dirname(__file__), "../..", "module"))
)

from utils import error, load_yaml
from validation import validate_item_structure


FILE = "type.yaml"
LANGUAGES = ["en", "fa"]


def main() -> None:  # Add the return type annotation here
    try:
        docs = load_yaml(FILE)

        for doc in docs:
            for item in doc.get("items", []):
                try:
                    validate_item_structure(item, LANGUAGES)
                except ValueError as e:
                    print(item)
                    error(str(e))

        print(FILE + ": Validation is successful")

    except Exception as e:
        error(f"An unexpected error occurred: {e}")


if __name__ == "__main__":
    main()  # Call the main function
