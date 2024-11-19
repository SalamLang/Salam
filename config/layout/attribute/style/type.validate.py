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

			if "type" not in item:
				print(item)
				error("type is required")

			if "generate_name" in item:
				if "reserved_values" not in item:
					print(item)
					error("reserved_values is required")
				if "text" not in item:
					print(item)
					error("text is required")
			else:
				if "reserved_values" in item:
					print(item)
					error("Why reserved_values exists for a non-generate_name item?")
				if "text" in item:
					print(item)
					error("Why text exists for a non-generate_name item?")
	
			if "text" in item:
				for lang in LANGUAGES:
					if lang not in item["text"]:
						print(item)
						error("text is required for " + lang + " language")

	print(FILE + ": Validation is successful")
