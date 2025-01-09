import os
import sys
import json
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

from lang import Lang
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("small")
    element.set_text(Lang.EN, "small")
    element.set_text(Lang.FA, "کوچک")

    element_dict = element.to_dict()

    j = json.dumps(element_dict, indent=4, ensure_ascii=False)
    print(j)
    open(Path(__file__).stem + ".json", "w", encoding="utf-8").write(j)
