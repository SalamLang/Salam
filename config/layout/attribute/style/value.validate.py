import yaml

FILE = "value.yaml"
LANGUAGES = ["en", "fa"]

file = open(FILE, "r", encoding="utf-8")
docs = yaml.safe_load_all(file)


def error(msg):
    print("Error: " + msg)
    exit(1)


if __name__ == "__main__":
	for doc in docs:
		for key in doc["items"]:
			items = doc["items"][key]
   
			if items is not None:
				for item in items:
					if "generate_name" not in item:
						print(item)
						error("generate_name is required")
			
					if "text" not in item:
						print(item)
						error("text is required")

					for lang in LANGUAGES:
						if lang not in item["text"]:
							print(item)
							error("text is required for " + lang + " language")

	print(FILE + ": Validation is successful")
