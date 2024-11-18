import yaml

SELECTED_LANGUAGE = 'fa'

COMMENT_BEGIN = "// ----------- BEGIN AUTO GENERATED ----------- //"
COMMENT_END = "// ----------- END AUTO GENERATED ----------- //"

def printify_type(item):
	global SELECTED_LANGUAGE
	
	idtext = item['id'].replace('AST_LAYOUT_TYPE_', '')
	
	return (
		f"ADD_TYPE" + 
  		f"(" + 
		f"{idtext}, "
		f"\"{idtext}\", "
		f"\"{idtext.lower()}\""
		f")\n"
	)

def printify_layout_type(item):
	global SELECTED_LANGUAGE

	def command(value):
		idtext = item['id'].replace('AST_LAYOUT_TYPE_', '')
  
		return (
			f"({item['id']}, "
			f"\"{idtext}\", "
			f"\"{idtext.lower()}\", "
			f"\"{item.get('generate_name', idtext.lower())}\", "
			f"\"{value}\", "
			f"{str(item.get('is_mother', False)).lower()}" +
			f")\n"
		)

	values = item['text'][SELECTED_LANGUAGE]
	
	if 'generate_name' not in item:
		if type(values) is str:
			return "ADD_LAYOUT_TYPE_HIDE" + command(values)
		else:
			result = ""
			for index, value in enumerate(values):
				print(index, value)
				if index == 0:
					result += "ADD_LAYOUT_TYPE_HIDE" + command(value)
				else:
					result += "ADD_LAYOUT_TYPE_REPEAT" + command(value)
			return result
	else:
		if type(values) is str:
			return "ADD_LAYOUT_TYPE" + command(values)
		else:
			result = ""
			for index, value in enumerate(values):
				print(index, value)
				if index == 0:
					result += "ADD_LAYOUT_TYPE" + command(value)
				else:
					result += "ADD_LAYOUT_TYPE_REPEAT" + command(value)
			return result

FILES = [
	{
		"input": "type.yaml",
		"output": "ast_type.h",
		"printify": printify_type,
	},
	{
		"input": "layout/type.yaml",
		"output": "ast_layout_type.h",
		"printify": printify_layout_type,
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
		# for line in lines:
		# 	if COMMENT_END in line:
		# 		break
		f.write("\n")

for file in FILES:
	print(file)
	sync_file(file)

