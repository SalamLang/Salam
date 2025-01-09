import json
from pathlib import Path

def to_string(element_dict):
    return json.dumps(element_dict, indent=4, ensure_ascii=False)


def print(element_str):
    print(element_str)


def save(element_str):
    open(Path(__file__).stem + ".json", "w", encoding="utf-8").write(
        element_str + "\n"
    )
