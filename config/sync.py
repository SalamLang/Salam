import json
import os
import sys
from typing import Any, Dict, List, Optional

import yaml
from module.utils import command_layout_style_type, command_layout_type

SELECTED_LANGUAGE = "fa"
if len(sys.argv) > 1:
    SELECTED_LANGUAGE = sys.argv[1]

print(f"Selected Language: {SELECTED_LANGUAGE}")

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
        "ADD_BLOCK_TYPE"
        + "("
        + f"{itemid}, "
        + f'"{idtext}", '
        + f'"{idtextlower}"'
        + ")\n"
    )


def prettify_layout_attribute_type(item: Dict[str, Any], group: Dict[str, Any]) -> str:
    """
    Generates a formatted string for a layout attribute type definition.

    :param item: A dictionary representing the layout attribute type item.
    :param group: A dictionary representing the group the item belongs to.
    :return: A formatted string for the layout attribute type.
    """
    global SELECTED_LANGUAGE

    values = item.get("text", {}).get(SELECTED_LANGUAGE, "")

    key_normal = "ADD_LAYOUT_ATTRIBUTE_TYPE"
    key_repeat = "ADD_LAYOUT_ATTRIBUTE_TYPE_REPEAT"

    if type(values) is not str:
        result = ""
        for index, value in enumerate(values):
            if index == 0:
                result += (
                    key_normal
                    + command_layout_style_type(
                        item, "AST_LAYOUT_ATTRIBUTE_TYPE_", value
                    )
                    + "\n"
                )
            else:
                result += (
                    key_repeat
                    + command_layout_style_type(
                        item, "AST_LAYOUT_ATTRIBUTE_TYPE_", value
                    )
                    + "\n"
                )
        return result
    else:
        return (
            key_normal
            + command_layout_style_type(item, "AST_LAYOUT_ATTRIBUTE_TYPE_", values)
            + "\n"
        )


def prettify_layout_attribute_style_global_value(
    item: Dict[str, Any], group: Dict[str, Any]
) -> str:
    """
    Generates a formatted string for a layout attribute style global value definition.

    :param item: A dictionary representing the style global value item.
    :param group: A dictionary representing the group the item belongs to.
    :return: A formatted string for the style global value.
    """
    global SELECTED_LANGUAGE

    itemid = item["id"]
    values = item.get("text", {}).get(SELECTED_LANGUAGE, "")

    key_normal = "ADD_LAYOUT_ATTRIBUTE_STYLE_GLOBAL_VALUE"
    key_repeat = "ADD_LAYOUT_ATTRIBUTE_STYLE_GLOBAL_VALUE_REPEAT"

    if type(values) is not str:
        result = ""
        for index, value in enumerate(values):
            if index == 0:
                result += key_normal
            else:
                result += key_repeat
            result += "(" + f"{itemid}, " + f'"{str(value)}"' + ")\n"

        return result
    else:
        return key_normal + "(" + f"{itemid}, " + f'"{str(values)}"' + ")\n"


def prettify_layout_attribute_style_type(
    item: Dict[str, Any], group: Dict[str, Any]
) -> str:
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
            return (
                key_hide
                + command_layout_style_type(
                    item, "AST_LAYOUT_ATTRIBUTE_STYLE_TYPE_", values
                )
                + "\n"
            )
        else:
            result = ""
            for index, value in enumerate(values):
                if index == 0:
                    result += (
                        key_hide
                        + command_layout_style_type(
                            item, "AST_LAYOUT_ATTRIBUTE_STYLE_TYPE_", value
                        )
                        + "\n"
                    )
                else:
                    result += (
                        key_repeat
                        + command_layout_style_type(
                            item, "AST_LAYOUT_ATTRIBUTE_STYLE_TYPE_", value
                        )
                        + "\n"
                    )
            return result
    else:
        if isinstance(values, str):
            return (
                key_main
                + command_layout_style_type(
                    item, "AST_LAYOUT_ATTRIBUTE_STYLE_TYPE_", values
                )
                + "\n"
            )
        else:
            result = ""
            for index, value in enumerate(values):
                if index == 0:
                    result += (
                        key_main
                        + command_layout_style_type(
                            item, "AST_LAYOUT_ATTRIBUTE_STYLE_TYPE_", value
                        )
                        + "\n"
                    )
                else:
                    result += (
                        key_repeat
                        + command_layout_style_type(
                            item, "AST_LAYOUT_ATTRIBUTE_STYLE_TYPE_", value
                        )
                        + "\n"
                    )
            return result


def prettify_layout_attribute_style_value_extern(
    item: Dict[str, Any], group: Dict[str, Any]
) -> str:
    """
    Generates a formatted string for layout attribute style values.

    :param key: A string representing the key name.
    :param items: A list of dictionaries representing the style value items.
    :return: A formatted string for the style values.
    """
    global SELECTED_LANGUAGE

    key = item.get("id", "unknown_key")  # Default value if id is missing
    if key == "unknown_key":
        print("Warning: Missing 'id' in item:", item)

    result = "extern const ast_layout_attribute_pair_t " + key + "[];\n"

    return result


def prettify_layout_attribute_style_value(
    item: Dict[str, Any], group: Dict[str, Any]
) -> str:
    """
    Generates a formatted string for layout attribute style values.

    :param key: A string representing the key name.
    :param items: A list of dictionaries representing the style value items.
    :return: A formatted string for the style values.
    """
    global SELECTED_LANGUAGE

    key = item.get("id", "unknown_key")  # Default value if id is missing
    if key == "unknown_key":
        print("Warning: Missing 'id' in item:", item)

    result = "const ast_layout_attribute_pair_t " + key + "[] = {\n"

    for item in item.get("childrens", []):
        item["generate_name"] = item["generate_name"].replace('"', '\\"')
        values = item.get("text", {}).get(SELECTED_LANGUAGE, "")

        if values is not None:
            if type(values) is str:
                values = values.replace('"', '\\"')
                result += '\t{"' + values + '", "' + item["generate_name"] + '"},\n'
            else:
                for value in values:
                    value = value.replace('"', '\\"')
                    result += '\t{"' + value + '", "' + item["generate_name"] + '"},\n'

    result += "\t{NULL, NULL},\n"
    result += "};\n"

    return result


def prettify_layout_attribute_value_extern(
    item: Dict[str, Any], group: Dict[str, Any]
) -> str:
    """
    Generates a formatted string for layout attribute values.

    :param key: A string representing the key name.
    :param items: A list of dictionaries representing the value items.
    :return: A formatted string for the values.
    """
    global SELECTED_LANGUAGE

    key = item.get("id", "unknown_key")  # Default value if id is missing
    if key == "unknown_key":
        print("Warning: Missing 'id' in item:", item)

    result = "extern const ast_layout_attribute_pair_t " + key + "[];\n"

    return result


def prettify_layout_attribute_value(item: Dict[str, Any], group: Dict[str, Any]) -> str:
    """
    Generates a formatted string for layout attribute values.

    :param key: A string representing the key name.
    :param items: A list of dictionaries representing the value items.
    :return: A formatted string for the values.
    """
    global SELECTED_LANGUAGE

    key = item.get("id", "unknown_key")  # Default value if id is missing
    if key == "unknown_key":
        print("Warning: Missing 'id' in item:", item)

    result = "const ast_layout_attribute_pair_t " + key + "[] = {\n"

    for item in item.get("childrens", []):
        item["generate_name"] = item["generate_name"].replace('"', '\\"')
        values = item.get("text", {}).get(SELECTED_LANGUAGE, "")

        if values is not None:
            if type(values) is str:
                values = values.replace('"', '\\"')
                result += '\t{"' + values + '", "' + item["generate_name"] + '"},\n'
            else:
                for value in values:
                    value = value.replace('"', '\\"')
                    result += '\t{"' + value + '", "' + item["generate_name"] + '"},\n'

    result += "\t{NULL, NULL},\n"
    result += "};\n"

    return result


def prettify_layout_attribute_style_state_type(
    item: Dict[str, Any], group: Dict[str, Any]
) -> str:
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
        result = ""
        index = 0
        for value in values:
            if index == 0:
                result += "ADD_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE"
            else:
                result += "ADD_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE_REPEAT"

            result += (
                "("
                + f"{itemid}, "
                + f'"{idtext}", '
                + f'"{idtextlower}", '
                + f'"{str(value)}", '
                + f'"{generate_name}"'
                + ")\n"
            )
            index += 1

        return result
    else:
        return (
            "ADD_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE"
            + "("
            + f"{itemid}, "
            + f'"{idtext}", '
            + f'"{idtextlower}", '
            + f'"{str(values)}", '
            + f'"{generate_name}"'
            + ")\n"
        )


def prettify_layout_type_attributes_values(
    item: Dict[str, Any], group: Dict[str, Any]
) -> str:
    """
    Generates a formatted string for a layout type definition.

    :param item: A dictionary representing the layout type item.
    :param group: A dictionary representing the group the item belongs to.
    :return: A formatted string for the layout type.
    """

    attributes = item.get("attributes", [])
    if attributes is None:
        attributes = []

    if "generate_name" in item:
        itemid = item["id"]
        itemid_lower = itemid.replace("AST_LAYOUT_TYPE_", "").lower()

        result = ""

        var_attrs = f"valid_attributes_{itemid_lower}"
        var_attrs_length = f"valid_attributes_{itemid_lower}_length"

        if len(attributes) > 0:
            result += f"ast_layout_attribute_type_t {var_attrs}[] = " + "{\n"
            for attr in attributes:
                result += "    " + attr + ",\n"
            result += "};\n"
            result += f"const size_t {var_attrs_length} = sizeof({var_attrs}) / sizeof({var_attrs}[0]);\n"

        return result
    else:
        return ""


def prettify_layout_type_attributes(item: Dict[str, Any], group: Dict[str, Any]) -> str:
    """
    Generates a formatted string for a layout type definition.

    :param item: A dictionary representing the layout type item.
    :param group: A dictionary representing the group the item belongs to.
    :return: A formatted string for the layout type.
    """

    attributes = item.get("attributes", [])
    if attributes is None:
        attributes = []

    if "generate_name" in item:
        itemid = item["id"]
        itemid_lower = itemid.replace("AST_LAYOUT_TYPE_", "").lower()

        result = f"else if (attribute->parent_node_type == {itemid}) " + "{\n"

        var_attrs = f"valid_attributes_{itemid_lower}"
        var_attrs_length = f"valid_attributes_{itemid_lower}_length"

        if len(attributes) > 0:
            result += (
                f"    if (is_attribute_type_in_array(attribute_key_type, {var_attrs}, {var_attrs_length})) "
                + "{\n"
            )
            result += "        return true;\n"
            result += "    }\n"

        result += "}"

        return result
    else:
        return ""


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
            return key_hide + command_layout_type(item, "AST_LAYOUT_TYPE_", values)
        else:
            result = ""
            for index, value in enumerate(values):
                if index == 0:
                    result += key_hide + command_layout_type(
                        item, "AST_LAYOUT_TYPE_", value
                    )
                else:
                    result += key_repeat + command_layout_type(
                        item, "AST_LAYOUT_TYPE_", value
                    )
            return result
    else:
        if isinstance(values, str):
            return key_main + command_layout_type(item, "AST_LAYOUT_TYPE_", values)
        else:
            result = ""
            for index, value in enumerate(values):
                if index == 0:
                    result += key_main + command_layout_type(
                        item, "AST_LAYOUT_TYPE_", value
                    )
                else:
                    result += key_repeat + command_layout_type(
                        item, "AST_LAYOUT_TYPE_", value
                    )
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
        "input": "layout/type.yaml",
        "output": "ast_layout_type_attributes.h",
        "prettify": prettify_layout_type_attributes,
    },
    {
        "input": "layout/type.yaml",
        "output": "ast_layout_type_attributes_values.h",
        "prettify": prettify_layout_type_attributes_values,
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
    {
        "input": "layout/attribute/style/value.yaml",
        "output": "ast_layout_attribute_style_value_extern.h",
        "prettify": prettify_layout_attribute_style_value_extern,
    },
    {
        "input": "layout/attribute/value.yaml",
        "output": "ast_layout_attribute_value.h",
        "prettify": prettify_layout_attribute_value,
    },
    {
        "input": "layout/attribute/value.yaml",
        "output": "ast_layout_attribute_value_extern.h",
        "prettify": prettify_layout_attribute_value_extern,
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

        started = False
        for line in lines:
            if started:
                f.write(line)
            elif COMMENT_END in line:
                started = True

        f.write("\n")


def generate_document() -> str:
    LAYOUT_TYPE_FILE = "layout/type.yaml"
    LAYOUT_ATTRIBUTE_TYPE_FILE = "layout/attribute/type.yaml"

    # Open and read the first file
    with open(LAYOUT_TYPE_FILE, "r", encoding="utf-8") as f:
        content = yaml.safe_load(f)
    items = content.get(
        "items", []
    )  # Safe get with default empty list if "items" is not found or None

    # Open and read the second file
    with open(LAYOUT_ATTRIBUTE_TYPE_FILE, "r", encoding="utf-8") as f:
        content = yaml.safe_load(f)
    attributes = content.get(
        "items", []
    )  # Safe get with default empty list if "items" is not found or None

    docs = '<div dir="rtl">\n\n# دستورات زبان برنامه نویسی سلام\n\n'

    def get_a_attr(attributes: List[Dict], attr_id: str) -> Optional[Dict]:
        for attr in attributes:
            if attr.get("id") == attr_id:
                return attr
        return None

    for item in items:
        id = item.get("id", "").replace("AST_LAYOUT_TYPE_", "").lower()

        descriptions = item.get("description", {})
        if descriptions is None or descriptions == "":
            descriptions = {}

        values = item.get("text", {}).get(SELECTED_LANGUAGE, "")
        first_value = values[0] if values else ""

        generate_name = item.get("generate_name", "")
        is_mother = item.get("is_mother", False)

        docs += f'<h2 id="{id}">دستور <code>{first_value}</code></h2>\n'
        docs += descriptions.get(SELECTED_LANGUAGE, "") + "\n"
        docs += (
            "این دستور "
            + (
                "مادر است و می تواند حامی دستوراتی به عنوان فرزند باشد."
                if is_mother
                else "فرزند است."
            )
            + "\n\n"
        )

        if generate_name != "":
            docs += "نام این دستور معادل دستور `" + generate_name + "` می‌باشد.\n\n"

        allowed_attributes = item.get("attributes", [])
        if allowed_attributes is None:
            allowed_attributes = []

        if len(allowed_attributes) > 0:
            docs += "این دستور دارای ویژگی‌های زیر می‌باشد:\n"
            docs += "\n"

            docs += "| نام ویژگی | توضیح | تولید |\n"
            docs += "|-----------|------------|-------|\n"
            for attr in allowed_attributes:
                attr_item = get_a_attr(attributes, attr)
                if attr_item is not None:
                    attr_generated_name = attr_item.get("generate_name", "")
                    attr_names = attr_item.get("text", {}).get(SELECTED_LANGUAGE, [])
                    attr_description = attr_item.get("description", {}).get(
                        SELECTED_LANGUAGE, ""
                    )
                    attr_names_str = "<br>".join(f"`{item}`" for item in attr_names)

                    docs += f"| {attr_names_str} | {attr_description} | `{attr_generated_name}` |\n"
            docs += "\n"

    docs += "\n</div>\n"

    return docs


if __name__ == "__main__":
    print("Start...")

    if len(FILES) == 0:
        print("No files to sync")
    else:
        for file in FILES:
            print(file)
            sync_file(file)
            convert_to_json(file)

        docs = generate_document()
        f = open("docs.md", "w", encoding="utf-8")
        f.write(docs)
        f.close()

        print("Done.")
