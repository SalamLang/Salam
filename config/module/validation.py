from typing import Any, Dict, List

from utils import error


def validate_item_structure(item: Dict[str, Any], languages: List[str]) -> None:
    """
    Validates the structure of an item against certain rules.

    :param item: The item dictionary to validate.
    :param languages: List of required language keys to check.
    """
    if "generate_name" not in item:
        error("generate_name is required")

    if "text" not in item:
        error("text is required")

    for lang in languages:
        if lang not in item["text"]:
            error(f"text is required for {lang} language")

    if "id" not in item:
        print(f"Warning: 'id' is missing in item: {item}")

    if "type" not in item:
        print(f"Warning: 'type' is missing in item: {item}")
