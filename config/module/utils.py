from typing import Any, Dict, List

import yaml


def load_yaml(file_path: str) -> List[Dict[str, Any]]:
    """
    Loads a YAML file and returns its contents as a list of dictionaries.
    Args:
        file_path (str): Path to the YAML file.
    Returns:
        List[Dict[str, Any]]: Parsed contents of the YAML file.
    """
    try:
        with open(file_path, "r", encoding="utf-8") as file:
            return list(yaml.safe_load_all(file))
    except FileNotFoundError:
        error(f"File not found: {file_path}")
        return []
    except yaml.YAMLError as e:
        error(f"Error parsing YAML file: {file_path}\n{e}")
        return []


def error(msg: str) -> None:
    """
    Prints an error message and exits the program.
    Args:
        msg (str): The error message to display.
    """
    print("Error: " + msg)
    exit(1)


def command_layout_type(item: Dict[str, Any], prefix: str, value: str) -> str:
    itemid = item["id"]
    idtext = itemid.replace(prefix, "")
    idtextlower = idtext.lower()
    generate_name = item.get("generate_name", idtext.lower())
    is_single = str(item.get("is_single", "false")).lower()

    return (
        f"({itemid}, "
        + f'"{idtext}", '
        + f'"{idtextlower}", '
        + f'"{generate_name}", '
        + f'"{value}", '
        + f"{is_single}"
        + ")\n"
    )


def command_layout_style_type(
    item: Dict[str, Any], prefix: str, value: str, extra: str = "none"
) -> str:
    """
    Generates a formatted command string based on the item and value provided.
    :param item: Dictionary containing item data.
    :param prefix: Prefix for the item ID replacement.
    :param value: The value to include in the command.
    :return: Formatted string for the command.
    """
    itemid = item.get("id", "")
    if not itemid:
        raise ValueError("Item must contain an 'id' key")

    idtext = itemid.replace(prefix, "")
    idtextlower = idtext.lower()

    generate_name = item.get("generate_name", idtextlower)

    type = str(item.get("type", "AST_LAYOUT_ATTRIBUTE_FILTER_STRING"))

    reserved_values = (
        str(item.get("reserved_values", "NULL")).lower()
        if item.get("reserved_values", "") != ""
        else "NULL"
    )

    if extra == "none":
        return (
            f"({itemid}, "
            f'"{idtext}", '
            f'"{idtextlower}", '
            f'"{value}", '
            f'"{generate_name}", '
            f"{type}, {reserved_values}, NULL)"
        )
    elif extra == "":
        return (
            f"({itemid}, "
            f'"{idtext}", '
            f'"{idtextlower}", '
            f'"{value}", '
            f'"{generate_name}", '
            f"{type}, {reserved_values}, NULL, value_handler_simple)"
        )
    else:
        return (
            f"({itemid}, "
            f'"{idtext}", '
            f'"{idtextlower}", '
            f'"{value}", '
            f'"{generate_name}", '
            f"{type}, {reserved_values}, NULL, {extra})"
        )


# Sample usage:
# if __name__ == "__main__":
#     file_path = "example.yaml"
#     items = load_yaml(file_path)
#     for item in items:
#         prefix = "prefix_"
#         value = "some_value"
#         print(command(item, prefix, value))
