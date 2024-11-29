import os
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


def validate_item_structure(item: Dict[str, Any], languages: List[str]) -> None:
    """
    Validates the structure of an item against certain rules.

    :param item: The item dictionary to validate.
    :param languages: List of required language keys to check.
    """
    if "id" not in item:
        raise ValueError("id is required and missed in an item")

    if "is_mother" in item:
        if item["is_mother"] not in [True, False]:
            raise ValueError("is_mother is invalid in an item")

    if "text" in item:
        for lang in languages:
            if lang not in item["text"]:
                raise ValueError(f"text is required for {lang} language")


def validate_item(item: Dict[str, Any], languages: List[str]) -> None:
    """
    Validates an item for required fields and their structure.

    :param item: The item to validate.
    :param languages: List of languages to check for text.
    """
    if "id" not in item:
        error("id is required and missed in an item")

    if "generate_name" not in item:
        error("generate_name is required")

    if "text" in item:
        for lang in languages:
            if lang not in item["text"]:
                error(f"text is required for {lang} language")


def command(item: Dict[str, Any], prefix: str, value: str) -> str:
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
