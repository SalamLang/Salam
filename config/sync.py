import yaml

SELECTED_LANGUAGE = "fa"

COMMENT_BEGIN = "// ----------- BEGIN AUTO GENERATED ----------- //"
COMMENT_END = "// ----------- END AUTO GENERATED ----------- //"

def printify_type(item, group):
    global SELECTED_LANGUAGE

    idtext = item["id"].replace("AST_TYPE_", "")

    return (
        f"ADD_TYPE" +
        f"(" +
        f"{item["id"]}, " +
        f"\"{idtext}\", " +
        f"\"{idtext.lower()}\"" +
        f")\n"
    )

def printify_block_type(item, group):
    global SELECTED_LANGUAGE

    idtext = item["id"].replace("AST_BLOCK_TYPE_", "")

    return (
        f"ADD_BLOCK_TYPE" +
          f"(" +
        f"{item["id"]}, " +
        f"\"{idtext}\", " +
        f"\"{idtext.lower()}\"" +
        f")\n"
    )

def printify_layout_attribute_type(item, group):
    global SELECTED_LANGUAGE

    idtext = item["id"].replace("AST_LAYOUT_ATTRIBUTE_TYPE_", "")
    values = item.get("text", {}).get(SELECTED_LANGUAGE, "")

    if type(values) is not str:
        return "" # TODO

    generate_name = item.get("generate_name", "")

    return (
        f"ADD_LAYOUT_ATTRIBUTE_TYPE" +
          f"(" +
        f"{item["id"]}, " +
        f"\"{idtext}\", " +
        f"\"{idtext.lower()}\", " +
        f"\"{generate_name}\", " +
        f"\"{str(values)}\"" +
        f")\n"
    )

def printify_layout_attribute_style_global_value(item, group):
    global SELECTED_LANGUAGE

    values = item.get("text", {}).get(SELECTED_LANGUAGE, "")

    if type(values) is not str:
        return "" # TODO

    return (
        f"ADD_LAYOUT_ATTRIBUTE_STYLE_GLOBAL_VALUE" +
          f"(" +
        f"\"{item["id"]}\", " +
        f"\"{str(values)}\"" +
        f")\n"
    )

def printify_layout_attribute_style_type(item, group):
    global SELECTED_LANGUAGE

    def command(value):
        idtext = item["id"].replace("AST_LAYOUT_ATTRIBUTE_STYLE_TYPE_", "")

        return (
            f"({item["id"]}, "
            f"\"{idtext}\", "
            f"\"{idtext.lower()}\", "
            f"\"{value}\", "
            f"\"{item.get("generate_name", idtext.lower())}\", "
            f"{str(item.get("type", "AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_STRING_ANY"))}, " +
            f"{str(item.get('reserved_values', 'NULL')).lower() if item.get('reserved_values', '') != '' else 'NULL'}, " +
            f"NULL" +
            f")\n"
        )

    values = item.get("text", {}).get(SELECTED_LANGUAGE, "")
    key_main = "ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE"
    key_hide = "ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE"
    key_repeat = "ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_REPEAT"

    if "generate_name" not in item:
        if type(values) is str:
            return key_hide + command(values)
        else:
            result = ""
            for index, value in enumerate(values):
                print(index, value)
                if index == 0:
                    result += key_hide + command(value)
                else:
                    result += key_repeat + command(value)
            return result
    else:
        if type(values) is str:
            return key_main + command(values)
        else:
            result = ""
            for index, value in enumerate(values):
                print(index, value)
                if index == 0:
                    result += key_main + command(value)
                else:
                    result += key_repeat + command(value)
            return result

def printify_layout_attribute_style_value(key, items):
    global SELECTED_LANGUAGE

    result = "const ast_layout_attribute_style_pair_t " + key + "[] = {\n"

    if items is not None:
        for item in items:
            item["generate_name"] = item["generate_name"].replace("\"", "\\\"")
            values = item.get("text", {}).get(SELECTED_LANGUAGE, "")

            if values is not None:
                if type(values) is str:
                    values = values.replace("\"", "\\\"")
                    result += "\t{\"" + values + "\", \"" + item["generate_name"] + "\"},\n"
                else:
                    for value in values:
                        value = value.replace("\"", "\\\"")
                        result += "\t{\"" + value + "\", \"" + item["generate_name"] + "\"},\n"

    result += "\t{NULL, NULL},\n"
    result += "};\n"

    return result

def printify_layout_attribute_style_state_type(item, group):
    global SELECTED_LANGUAGE

    idtext = item["id"].replace("AST_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE_", "")
    values = item.get("text", {}).get(SELECTED_LANGUAGE, "")

    if type(values) is not str:
        return "" # TODO

    return (
        f"ADD_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE" +
          f"(" +
        f"{item["id"]}, " +
        f"\"{idtext}\", " +
        f"\"{idtext.lower()}\", " +
        f"\"{str(values)}\", " +
        f"\"{item.get("generate_name", "")}\"" +
        f")\n"
    )

def printify_layout_type(item, group):
    global SELECTED_LANGUAGE

    def command(value):
        idtext = item["id"].replace("AST_LAYOUT_TYPE_", "")

        return (
            f"({item["id"]}, " +
            f"\"{idtext}\", " +
            f"\"{idtext.lower()}\", " +
            f"\"{item.get("generate_name", idtext.lower())}\", " +
            f"\"{value}\", " +
            f"{str(item.get("is_mother", False)).lower()}" +
            f")\n"
        )

    values = item.get("text", {}).get(SELECTED_LANGUAGE, "")
    key_main = "ADD_LAYOUT_TYPE"
    key_hide = "ADD_LAYOUT_TYPE_HIDE"
    key_repeat = "ADD_LAYOUT_TYPE_REPEAT"

    if "generate_name" not in item:
        if type(values) is str:
            return key_hide + command(values)
        else:
            result = ""
            for index, value in enumerate(values):
                print(index, value)
                if index == 0:
                    result += key_hide + command(value)
                else:
                    result += key_repeat + command(value)
            return result
    else:
        if type(values) is str:
            return key_main + command(values)
        else:
            result = ""
            for index, value in enumerate(values):
                print(index, value)
                if index == 0:
                    result += key_main + command(value)
                else:
                    result += key_repeat + command(value)
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
        "input": "layout/attribute/type.yaml",
        "output": "ast_layout_attribute_type.h",
        "printify": printify_layout_attribute_type,
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
    {
        "input": "layout/attribute/style/value.yaml",
        "output": "ast_layout_attribute_style_value.h",
        "printify": printify_layout_attribute_style_value,
    },
]


def sync_file(file):
    print("Syncing file: " + file["input"] + " -> " + file["output"])

    output_filename = "../src/config/" + file["output"]
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
                    if isinstance(group["items"], dict):
                        f.write(file["printify"](item, group["items"][item]) + "\n")
                    elif isinstance(group["items"], list):
                        f.write(file["printify"](item, None) + "\n")
                    else:
                        f.write(file["printify"](item, None) + "\n")
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
