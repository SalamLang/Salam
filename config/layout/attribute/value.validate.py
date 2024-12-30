import os
import sys

sys.path.append(
    os.path.abspath(os.path.join(os.path.dirname(__file__), "../..", "module"))
)

from module.utils import error, load_yaml
from module.validation import validate_item_structure

FILE = "value.yaml"
LANGUAGES = ["en", "fa"]


def main() -> None:
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
    main()
