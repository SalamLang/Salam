import yaml

FILE = "type.yaml"
LANGUAGES = ["en", "fa"]

file = open(FILE, "r", encoding="utf-8")
docs = yaml.safe_load_all(file)


def error(msg):
    print("Error: " + msg)
    exit(1)


for doc in docs:
    for item in doc["items"]:
        if "id" not in item:
            error("id is required and missed in an item")

        if "text" in item:
            for lang in LANGUAGES:
                if lang not in item["text"]:
                    error("text is required for " + lang + " language")

print(FILE + ": Validation is successful")
