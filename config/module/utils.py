from typing import Dict, List, Any
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
    except yaml.YAMLError as e:
        error(f"Error parsing YAML file: {file_path}\n{e}")

def error(msg: str) -> None:
    """
    Prints an error message and exits the program.
    Args:
        msg (str): The error message to display.
    """
    print("Error: " + msg)
    exit(1)

def command(item: Dict[str, any], prefix: str, value: str) -> str:
    """
    Generates a formatted command string based on the item and value provided.

    :param item: Dictionary containing item data.
    :param prefix: Prefix for the item ID replacement.
    :param value: The value to include in the command.
    :return: Formatted string for the command.
    """
    itemid = item["id"]
    idtext = itemid.replace(prefix, "")
    idtextlower = idtext.lower()

    generate_name = item.get("generate_name", idtextlower)
    type = str(item.get("type", "AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_STRING_ANY"))
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