import yaml

SELECTED_LANGUAGE = 'fa'

COMMENT_BEGIN = "// ----------- BEGIN AUTO GENERATED ----------- //"
COMMENT_END = "// ----------- END AUTO GENERATED ----------- //"

def printify_type(item):
	return (
		f"AST_TYPE({item['id']}, "
		f"\"{item['id'].replace('AST_TYPE_', '')}\", "
		f"\"{item['id'].replace('AST_TYPE_', '').lower()}\""
		f")"
	)
 
def printify_layout_type(item):
	# if item['text'][SELECTED_LANGUAGE] is string so use it, otherwise foreach on it and use REPEAT for second and the next one (not first)
	# If 'generate_name' is missing, use ADD_LAYOUT_TYPE_HIDE
	return (
		f"ADD_LAYOUT_TYPE_HIDE({item['id']}, "
		f"\"{item['id'].replace('AST_LAYOUT_TYPE_', '')}\", "
		f"\"{item['id'].replace('AST_LAYOUT_TYPE_', '').lower()}\", "
		f"\"{item['text'][SELECTED_LANGUAGE]}\", "
		f"{str(item.get('is_mother', False)).lower()})"
		
		if 'generate_name' not in item else (
			# Use ADD_LAYOUT_TYPE_REPEAT if 'repeat' is true
			f"ADD_LAYOUT_TYPE_REPEAT({item['id']}, "
			f"\"{item['id'].replace('AST_LAYOUT_TYPE_', '')}\", "
			f"\"{item['generate_name']}\", "
			f"\"{item['text']['en']}\", "
			f"\"{item['text']['fa']}\", "
			f"{str(item.get('is_mother', False)).lower()})"
			
			if item.get('repeat', False) else (
				# Default case for ADD_LAYOUT_TYPE
				f"ADD_LAYOUT_TYPE({item['id']}, "
				f"\"{item['id'].replace('AST_LAYOUT_TYPE_', '')}\", "
				f"\"{item['generate_name']}\", "
				f"\"{item['text']['en']}\", "
				f"\"{item['text']['fa']}\", "
				f"{str(item.get('is_mother', False)).lower()})"
			)
		)
	)
FILES = [
    {
        "input": "type.yaml",
        "output": "ast_type.h",
        "printify": lambda item: (
            f"AST_TYPE({item['id']}, "
            f"\"{item['id'].replace('AST_TYPE_', '')}\", "
            f"\"{item['id'].replace('AST_TYPE_', '').lower()}\""
            f")"
        ),
    },
    {
        "input": "layout/type.yaml",
        "output": "ast_layout_type.h",
        "printify": lambda item: (
            # if item['text'][SELECTED_LANGUAGE] is string so use it, otherwise foreach on it and use REPEAT for second and the next one (not first)
            # If 'generate_name' is missing, use ADD_LAYOUT_TYPE_HIDE
            f"ADD_LAYOUT_TYPE_HIDE({item['id']}, "
            f"\"{item['id'].replace('AST_LAYOUT_TYPE_', '')}\", "
            f"\"{item['id'].replace('AST_LAYOUT_TYPE_', '').lower()}\", "
            f"\"{item['text'][SELECTED_LANGUAGE]}\", "
            f"{str(item.get('is_mother', False)).lower()})"
            
            if 'generate_name' not in item else (
                # Use ADD_LAYOUT_TYPE_REPEAT if 'repeat' is true
                f"ADD_LAYOUT_TYPE_REPEAT({item['id']}, "
                f"\"{item['id'].replace('AST_LAYOUT_TYPE_', '')}\", "
                f"\"{item['generate_name']}\", "
                f"\"{item['text']['en']}\", "
                f"\"{item['text']['fa']}\", "
                f"{str(item.get('is_mother', False)).lower()})"
                
                if item.get('repeat', False) else (
                    # Default case for ADD_LAYOUT_TYPE
                    f"ADD_LAYOUT_TYPE({item['id']}, "
                    f"\"{item['id'].replace('AST_LAYOUT_TYPE_', '')}\", "
                    f"\"{item['generate_name']}\", "
                    f"\"{item['text']['en']}\", "
                    f"\"{item['text']['fa']}\", "
                    f"{str(item.get('is_mother', False)).lower()})"
                )
            )
        ),
    },
]


def sync_file(file):
	print("Syncing file: " + file["input"] + " -> " + file["output"])
	
	output_filename = "../src/" + file["output"]
	input_filename = file["input"]

	f = open(input_filename, "r", encoding="utf-8")
	content = yaml.safe_load_all(f)

	with open(output_filename, "r", encoding="utf-8") as f:
		lines = f.readlines()
	
	with open(output_filename, "w", encoding="utf-8") as f:
		for line in lines:
			if COMMENT_BEGIN in line:
				break
			f.write(line)
		f.write(COMMENT_BEGIN + "\n")
		for group in content:
			for item in group["items"]:
				if "printify" in file:
					f.write(file["printify"](item) + "\n")
				else:
					f.write(str(item) + "\n")
		f.write(COMMENT_END + "\n")
		for line in lines:
			if COMMENT_END in line:
				break
		f.write("\n")

for file in FILES:
	print(file)
	sync_file(file)

