import os
import sys

sys.path.append(
    os.path.abspath(os.path.join(os.path.dirname(__file__), "../../../module"))
)

from utils import error, load_yaml
from validation import validate_item_structure

FILE = "value.yaml"
LANGUAGES = ["en", "fa"]

if __name__ == "__main__":
    try:
        docs = load_yaml(FILE)

        for doc in docs:
            for key in doc.get("items", {}):
                items = doc["items"][key]

                if items is not None:
                    for item in items:
                        validate_item_structure(item, LANGUAGES)

        print(FILE + ": Validation is successful")

    except Exception as e:
        error(f"An unexpected error occurred: {e}")
