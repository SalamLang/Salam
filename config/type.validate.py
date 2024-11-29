from typing import Dict, List
from module.utils import load_yaml, error

FILE = "type.yaml"
LANGUAGES = ["en", "fa"]

docs: List[Dict] = load_yaml(FILE)

if __name__ == "__main__":
    for doc in docs:
        for item in doc["items"]:
            if "id" not in item:
                print(item)
                error("id is required and missed in an item")

            if "text" in item:
                for lang in LANGUAGES:
                    if lang not in item["text"]:
                        print(item)
                        error("text is required for " + lang + " language")

    print(FILE + ": Validation is successful")
