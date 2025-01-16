import json
from pathlib import Path
from typing import Any, Dict


class Prebuild:
    @classmethod
    def to_string(cls, element_dict: Dict[str, Any]) -> str:
        """Convert a dictionary to a JSON string with indentation."""
        return json.dumps(element_dict, indent=4, ensure_ascii=False)

    @classmethod
    def print(cls, element_str: str) -> None:
        """Print a given string."""
        print(element_str)

    @classmethod
    def save(cls, element_str: str, file: str) -> None:
        """Save a string to a JSON file."""
        Path(file).with_suffix(".json").write_text(element_str + "\n", encoding="utf-8")

    @classmethod
    def idize(cls, value: str) -> str:
        """Convert a string into a normalized identifier."""
        return (
            value.replace(" ", "_")
            .replace("-", "_")
            .replace("/", "_")
            .replace(".", "_")
            .replace("__", "_")
        )
