import json
from pathlib import Path

class Build:
    @classmethod
    def to_string(self, element_dict):
        return json.dumps(element_dict, indent=4, ensure_ascii=False)

    @classmethod
    def print(self, element_str):
        print(element_str)

    @classmethod
    def save(self, element_str, file):
        open(Path(file).stem + ".json", "w", encoding="utf-8").write(
            element_str + "\n"
        )
