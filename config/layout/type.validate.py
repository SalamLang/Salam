import yaml

FILE = "type.yaml"
LANGUAGES = ["en", "fa"]

file = open(FILE, "r", encoding="utf-8")
docs = yaml.safe_load_all(file)


def error(msg):
    print("Error: " + msg)
    exit(1)


if __name__ == "__main__":
    for doc in docs:
        for item in doc["items"]:
            if "id" not in item:
                print(item)
                error("id is required and missed in an item")

            if "is_mother" in item:
                if item["is_mother"] != True and item["is_mother"] != False:
                    print(item)
                    error("is_mother is invalid in an item")

            if "text" in item:
                for lang in LANGUAGES:
                    if lang not in item["text"]:
                        print(item)
                        error("text is required for " + lang + " language")

    print(FILE + ": Validation is successful")
