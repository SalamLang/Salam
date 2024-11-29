from typing import Dict, List

from module.utils import error, load_yaml
from module.validation import validate_item_structure

FILE = "language.yaml"
LANGUAGES = ["en", "fa"]

docs: List[Dict] = load_yaml(FILE)

if __name__ == "__main__":
    for doc in docs:
        for item in doc.get("items", []):
            try:
                validate_item_structure(item, LANGUAGES)
            except ValueError as e:
                print(item)
                error(str(e))

    print(FILE + ": Validation is successful")
