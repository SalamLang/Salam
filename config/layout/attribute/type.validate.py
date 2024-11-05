import yaml

FILE = "type.yaml"
LANGUAGES = ["en", "fa"]
ALL_ATTRS = ["id", "generate_name", "text"]

file = open(FILE, "r", encoding="utf-8")
docs = yaml.safe_load_all(file)


def error(msg):
    print("Error: " + msg)
    exit(1)


for doc in docs:
    for item in doc["items"]:
        if "id" not in item:
            print(item)
            error("id is required and missed in an item")

        if "generate_name" not in item:
            if "text" in item:
                for lang in LANGUAGES:
                    if lang not in item["text"]:
                        print(item)
                        error("text is required for " + lang + " language")

print(FILE + ": Validation is successful")
