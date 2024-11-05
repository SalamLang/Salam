import yaml

FILE = "language.yaml"
LANGUAGES = ["en", "fa"]

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

        if "name" not in item:
            print(item)
            error("name is required and missed in an item")

        if "local_name" not in item:
            print(item)
            error("local_name is required and missed in an item")

print(FILE + ": Validation is successful")
