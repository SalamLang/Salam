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
        return []  # Ensure a return value
    except yaml.YAMLError as e:
        error(f"Error parsing YAML file: {file_path}\n{e}")
        return []  # Ensure a return value


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
    is_mother = str(item.get("is_mother", "false")).lower()

    return (
        f"({itemid}, "
        + f'"{idtext}", '
        + f'"{idtextlower}", '
        + f'"{generate_name}", '
        + f'"{value}", '
        + f"{is_mother}"
        + f")\n"
    )


def command_layout_style_type(item: Dict[str, Any], prefix: str, value: str) -> str:
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

    # Get generate_name or fallback to idtextlower if not found
    generate_name = item.get("generate_name", idtextlower)

    # Default to "AST_LAYOUT_ATTRIBUTE_STYLE_FILTER_STRING_ANY" if "type" is not found
    type = str(item.get("type", "AST_LAYOUT_ATTRIBUTE_STYLE_FILTER_STRING_ANY"))

    # Handle reserved_values, default to "NULL" if empty
    reserved_values = (
        str(item.get("reserved_values", "NULL")).lower()
        if item.get("reserved_values", "") != ""
        else "NULL"
    )

    return (
        f"({itemid}, "
        f'"{idtext}", '
        f'"{idtextlower}", '
        f'"{value}", '
        f'"{generate_name}", '
        f"{type}, {reserved_values}, NULL)"
    )


# # Sample usage:
# if __name__ == "__main__":
#     # Example of loading a YAML file
#     file_path = "example.yaml"  # Replace with your actual YAML file path
#     items = load_yaml(file_path)

#     for item in items:
#         # Example command generation for each item in the loaded YAML
#         prefix = "prefix_"
#         value = "some_value"
#         print(command(item, prefix, value))
