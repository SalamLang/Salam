import os
import yaml
import json
from typing import Any, Dict, List
from module.utils import command


SELECTED_LANGUAGE = "fa"

COMMENT_BEGIN = "// ----------- BEGIN AUTO GENERATED ----------- //"
COMMENT_END = "// ----------- END AUTO GENERATED ----------- //"
JSON_DIR = "json/"

def prettify_type(item: Dict[str, Any], group: Dict[str, Any]) -> str:
    """
    Generates a formatted string for a type definition.

    :param item: A dictionary representing the type item.
    :param group: A dictionary representing the group the item belongs to.
    :return: A formatted string for the type.
    """
    global SELECTED_LANGUAGE

    itemid = item["id"]
    idtext = itemid.replace("AST_TYPE_", "")
    
    return f"ADD_TYPE(" f"{itemid}, " f'"{idtext}", ' f'"{idtext.lower()}"' f")\n"


def prettify_block_type(item: Dict[str, Any], group: Dict[str, Any]) -> str:
    """
    Generates a formatted string for a block type definition.

    :param item: A dictionary representing the block type item.
    :param group: A dictionary representing the group the item belongs to.
    :return: A formatted string for the block type.
    """
    global SELECTED_LANGUAGE

    itemid = item["id"]
    idtext = itemid.replace("AST_BLOCK_TYPE_", "")
    idtextlower = idtext.lower()

    return (
        f"ADD_BLOCK_TYPE"
        + f"("
        + f"{itemid}, "
        + f'"{idtext}", '
        + f'"{idtextlower}"'
        + f")\n"
    )


def prettify_layout_attribute_type(item: Dict[str, Any], group: Dict[str, Any]) -> str:
    """
    Generates a formatted string for a layout attribute type definition.

    :param item: A dictionary representing the layout attribute type item.
    :param group: A dictionary representing the group the item belongs to.
    :return: A formatted string for the layout attribute type.
    """
    global SELECTED_LANGUAGE

    itemid = item["id"]
    idtext = itemid.replace("AST_LAYOUT_ATTRIBUTE_TYPE_", "")
    idtextlower = idtext.lower()

    values = item.get("text", {}).get(SELECTED_LANGUAGE, "")

    if type(values) is not str:
        return ""  # TODO

    generate_name = item.get("generate_name", "")

    return (
        f"ADD_LAYOUT_ATTRIBUTE_TYPE"
        + f"("
        + f"{itemid}, "
        + f'"{idtext}", '
        + f'"{idtextlower}", '
        + f'"{generate_name}", '
        + f'"{str(values)}"'
        + f")\n"
    )


def prettify_layout_attribute_style_global_value(item: Dict[str, Any], group: Dict[str, Any]) -> str:
    """
    Generates a formatted string for a layout attribute style global value definition.

    :param item: A dictionary representing the style global value item.
    :param group: A dictionary representing the group the item belongs to.
    :return: A formatted string for the style global value.
    """
    global SELECTED_LANGUAGE

    itemid = item["id"]
    values = item.get("text", {}).get(SELECTED_LANGUAGE, "")

    if type(values) is not str:
        return ""  # TODO
    
    return (
        f"ADD_LAYOUT_ATTRIBUTE_STYLE_GLOBAL_VALUE"
        + f"("
        + f'{itemid}, '
        + f'"{str(values)}"'
        + f")\n"
    )


def prettify_layout_attribute_style_type(item: Dict[str, Any], group: Dict[str, Any]) -> str:
    """
    Generates a formatted string for a layout attribute style type definition.

    :param item: A dictionary representing the style type item.
    :param group: A dictionary representing the group the item belongs to.
    :return: A formatted string for the layout attribute style type.
    """
    global SELECTED_LANGUAGE

    values = item.get("text", {}).get(SELECTED_LANGUAGE, "")
    key_main = "ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE"
    key_hide = "ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE"
    key_repeat = "ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_REPEAT"

    if "generate_name" not in item:
        if isinstance(values, str):
            return key_hide + command(item, "AST_LAYOUT_ATTRIBUTE_STYLE_TYPE_", values)
        else:
            result = ""
            for index, value in enumerate(values):
                if index == 0:
                    result += key_hide + command(item, "AST_LAYOUT_ATTRIBUTE_STYLE_TYPE_", value)
                else:
                    result += key_repeat + command(item, "AST_LAYOUT_ATTRIBUTE_STYLE_TYPE_", value)
            return result
    else:
        if isinstance(values, str):
            return key_main + command(item, "AST_LAYOUT_ATTRIBUTE_STYLE_TYPE_", values)
        else:
            result = ""
            for index, value in enumerate(values):
                if index == 0:
                    result += key_main + command(item, "AST_LAYOUT_ATTRIBUTE_STYLE_TYPE_", value)
                else:
                    result += key_repeat + command(item, "AST_LAYOUT_ATTRIBUTE_STYLE_TYPE_", value)
            return result



def prettify_layout_attribute_style_value(key: str, items: List[Dict[str, Any]]) -> str:
    """
    Generates a formatted string for layout attribute style values.

    :param key: A string representing the key name.
    :param items: A list of dictionaries representing the style value items.
    :return: A formatted string for the style values.
    """
    global SELECTED_LANGUAGE

    result = "const ast_layout_attribute_style_pair_t " + key + "[] = {\n"

    if items is not None:
        for item in items:
            item["generate_name"] = item["generate_name"].replace('"', '\\"')
            values = item.get("text", {}).get(SELECTED_LANGUAGE, "")

            if values is not None:
                if type(values) is str:
                    values = values.replace('"', '\\"')
                    result += '\t{"' + values + '", "' + item["generate_name"] + '"},\n'
                else:
                    for value in values:
                        value = value.replace('"', '\\"')
                        result += (
                            '\t{"' + value + '", "' + item["generate_name"] + '"},\n'
                        )

    result += "\t{NULL, NULL},\n"
    result += "};\n"

    return result


def prettify_layout_attribute_style_state_type(item: Dict[str, Any], group: Dict[str, Any]) -> str:
    """
    Generates a formatted string for a layout attribute style state type definition.

    :param item: A dictionary representing the style state type item.
    :param group: A dictionary representing the group the item belongs to.
    :return: A formatted string for the style state type.
    """
    global SELECTED_LANGUAGE

    itemid = item["id"]
    idtext = itemid.replace("AST_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE_", "")
    idtextlower = idtext.lower()
    values = item.get("text", {}).get(SELECTED_LANGUAGE, "")
    generate_name = item.get("generate_name", "")

    if type(values) is not str:
        return ""  # TODO

    return (
        f"ADD_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE"
        + f"("
        + f"{itemid}, "
        + f'"{idtext}", '
        + f'"{idtextlower}", '
        + f'"{str(values)}", '
        + f'"{generate_name}"'
        + f")\n"
    )


def prettify_layout_type(item: Dict[str, Any], group: Dict[str, Any]) -> str:
    """
    Generates a formatted string for a layout type definition.

    :param item: A dictionary representing the layout type item.
    :param group: A dictionary representing the group the item belongs to.
    :return: A formatted string for the layout type.
    """
    global SELECTED_LANGUAGE

    values = item.get("text", {}).get(SELECTED_LANGUAGE, "")
    key_main = "ADD_LAYOUT_TYPE"
    key_hide = "ADD_LAYOUT_TYPE_HIDE"
    key_repeat = "ADD_LAYOUT_TYPE_REPEAT"

    if "generate_name" not in item:
        if isinstance(values, str):
            return key_hide + command(item, "AST_LAYOUT_TYPE_", values)
        else:
            result = ""
            for index, value in enumerate(values):
                if index == 0:
                    result += key_hide + command(item, "AST_LAYOUT_TYPE_", value)
                else:
                    result += key_repeat + command(item, "AST_LAYOUT_TYPE_", value)
            return result
    else:
        if isinstance(values, str):
            return key_main + command(item, "AST_LAYOUT_TYPE_", values)
        else:
            result = ""
            for index, value in enumerate(values):
                if index == 0:
                    result += key_main + command(item, "AST_LAYOUT_TYPE_", value)
                else:
                    result += key_repeat + command(item, "AST_LAYOUT_TYPE_", value)
            return result



FILES = [
    {
        "input": "type.yaml",
        "output": "ast_type.h",
        "prettify": prettify_type,
    },
    {
        "input": "block.yaml",
        "output": "ast_block_type.h",
        "prettify": prettify_block_type,
    },
    {
        "input": "layout/type.yaml",
        "output": "ast_layout_type.h",
        "prettify": prettify_layout_type,
    },
    {
        "input": "layout/attribute/type.yaml",
        "output": "ast_layout_attribute_type.h",
        "prettify": prettify_layout_attribute_type,
    },
    {
        "input": "layout/attribute/style/global_value.yaml",
        "output": "ast_layout_attribute_style_global.h",
        "prettify": prettify_layout_attribute_style_global_value,
    },
    {
        "input": "layout/attribute/style/state.yaml",
        "output": "ast_layout_attribute_style_state_type.h",
        "prettify": prettify_layout_attribute_style_state_type,
    },
    {
        "input": "layout/attribute/style/type.yaml",
        "output": "ast_layout_attribute_style_type.h",
        "prettify": prettify_layout_attribute_style_type,
    },
    {
        "input": "layout/attribute/style/value.yaml",
        "output": "ast_layout_attribute_style_value.h",
        "prettify": prettify_layout_attribute_style_value,
    },
]


def convert_to_json(file: Dict[str, Any]) -> None:
    """
    Converts a YAML file to a JSON file.

    :param file: A dictionary containing 'input' and 'output' file paths.
    """
    global JSON_DIR
    try:
        with open(file["input"], "r", encoding="utf-8") as yaml_file:
            data = yaml.safe_load(yaml_file)
        
        json_filename = JSON_DIR + file["input"].replace(".yaml", ".json")

        directory = os.path.dirname(json_filename)
        if not os.path.exists(directory):
            os.makedirs(directory)
        
        with open(json_filename, "w", encoding="utf-8") as json_file:
            json.dump(data, json_file, ensure_ascii=False, indent=4)

        print(f"Converted {file['input']} to {file['output']}")
    except Exception as e:
        print(f"Error converting {file['input']} to {file['output']}: {e}")


def sync_file(file: Dict[str, Any]) -> None:
    print("Syncing file: " + file["input"] + " -> " + file["output"])

    output_filename = "../src/generated-config/" + file["output"]
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

        print("file: ", file)
        for group in content:
            print("group: ", group)
            for item in group["items"]:
                print("item: ", item)
                
                if "prettify" in file:
                    print("prettify exists in the file")
                    
                    if isinstance(group["items"], dict):
                        f.write(file["prettify"](item, group["items"][item]) + "\n")
                    elif isinstance(group["items"], list):
                        f.write(file["prettify"](item, None) + "\n")
                    else:
                        f.write(file["prettify"](item, None) + "\n")
                else:
                    print("prettify do not exists in the file")
                    
                    f.write(str(item) + "\n")
        
        f.write(COMMENT_END + "\n")
        
        f.write("\n")


if __name__ == "__main__":
    if len(FILES) == 0:
        print("No files to sync")

    for file in FILES:
        print(file)
        sync_file(file)
        convert_to_json(file)
