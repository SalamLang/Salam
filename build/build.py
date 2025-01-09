import json
from pathlib import Path

def to_string(element_dict):
    return json.dumps(element_dict, indent=4, ensure_ascii=False)


def print(element_dict):
    print(to_string(element_dict))


def save(element_dict):
    open(Path(__file__).stem + ".json", "w", encoding="utf-8").write(
        to_string(element_dict) + "\n"
    )
