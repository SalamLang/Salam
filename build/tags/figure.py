import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

import build
from type import Type
from lang import Lang
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("figure")
    element.set_text(Lang.EN, "figure")
    element.set_text(Lang.FA, "فیگور")

    element_dict = element.to_dict()

    element_str = build.string(element_dict)
    build.print(element_str)
    build.save(element_str)
