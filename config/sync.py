import yaml

SELECTED_LANGUAGE = "fa"

COMMENT_BEGIN = "// ----------- BEGIN AUTO GENERATED ----------- //"
COMMENT_END = "// ----------- END AUTO GENERATED ----------- //"

def printify_type(item):
	global SELECTED_LANGUAGE
	
	idtext = item["id"].replace("AST_TYPE_", "")
	
	return (
		f"ADD_TYPE" + 
  		f"(" + 
		f"{item["id"]}, "
		f"\"{idtext}\", "
		f"\"{idtext.lower()}\""
		f")\n"
	)

def printify_block_type(item):
	global SELECTED_LANGUAGE
	
	idtext = item["id"].replace("AST_BLOCK_TYPE_", "")
	
	return (
		f"ADD_BLOCK_TYPE" + 
  		f"(" + 
		f"{item["id"]}, "
		f"\"{idtext}\", "
		f"\"{idtext.lower()}\""
		f")\n"
	)

def printify_layout_attribute_style_global_value(item):
	global SELECTED_LANGUAGE
	
	values = item["text"][SELECTED_LANGUAGE]

	if type(values) is not str:
		return "" # TODO

	return (
		f"ADD_LAYOUT_ATTRIBUTE_STYLE_GLOBAL_VALUE" + 
  		f"(" + 
		f"\"{item["id"]}\", "
		f"\"{str(values)}\""
		f")\n"
	)
 
def printify_layout_attribute_style_type(item):
	global SELECTED_LANGUAGE

	def command(value):
		idtext = item["id"].replace("AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_", "")
  
		return (
			f"({item["id"]}, "
			f"\"{idtext}\", "
			f"\"{idtext.lower()}\", "
			f"\"{item.get("generate_name", idtext.lower())}\", "
			f"\"{value}\", "
			f"{str(item.get("is_mother", False)).lower()}" +
			f")\n"
		)

	values = item["text"][SELECTED_LANGUAGE]
	
	if "generate_name" not in item:
		if type(values) is str:
			return "ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE" + command(values)
		else:
			result = ""
			for index, value in enumerate(values):
				print(index, value)
				if index == 0:
					result += "ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE" + command(value)
				else:
					result += "ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_REPEAT" + command(value)
			return result
	else:
		if type(values) is str:
			return "ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE" + command(values)
		else:
			result = ""
			for index, value in enumerate(values):
				print(index, value)
				if index == 0:
					result += "ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE" + command(value)
				else:
					result += "ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_REPEAT" + command(value)
			return result
 
 def printify_layout_attribute_style_state_type(item):
	global SELECTED_LANGUAGE
	
	idtext = item["id"].replace("AST_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE_", "")
	values = item.get("text", {}).get(SELECTED_LANGUAGE, "")

	if type(values) is not str:
		return "" # TODO

	return (
		f"ADD_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE" + 
  		f"(" + 
		f"{item["id"]}, "
		f"\"{idtext}\", "
		f"\"{idtext.lower()}\", "
		f"\"{str(values)}\", "
		f"\"{item.get("generate_name", "")}\""
		f")\n"
	)

def printify_layout_type(item):
	global SELECTED_LANGUAGE

	def command(value):
		idtext = item["id"].replace("AST_LAYOUT_TYPE_", "")
  
		return (
			f"({item["id"]}, "
			f"\"{idtext}\", "
			f"\"{idtext.lower()}\", "
			f"\"{item.get("generate_name", idtext.lower())}\", "
			f"\"{value}\", "
			f"{str(item.get("is_mother", False)).lower()}" +
			f")\n"
		)

	values = item["text"][SELECTED_LANGUAGE]
	
	if "generate_name" not in item:
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
		"input": "block.yaml",
		"output": "ast_block_type.h",
		"printify": printify_block_type,
	},
  	{
		"input": "layout/type.yaml",
		"output": "ast_layout_type.h",
		"printify": printify_layout_type,
	},
	{
		"input": "layout/attribute/style/global_value.yaml",
		"output": "ast_layout_attribute_style_global.h",
		"printify": printify_layout_attribute_style_global_value,
	},
	{
		"input": "layout/attribute/style/state.yaml",
		"output": "ast_layout_attribute_style_state_type.h",
		"printify": printify_layout_attribute_style_state_type,
	},
	{
		"input": "layout/attribute/style/type.yaml",
		"output": "ast_layout_attribute_style_type.h",
		"printify": printify_layout_attribute_style_type,
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

