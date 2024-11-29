import sys
import os
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'module')))

from utils import load_yaml, error, validate_item_structure

FILE = "type.yaml"
LANGUAGES = ["en", "fa"]

if __name__ == "__main__":
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
