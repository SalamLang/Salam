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
    element.set_generate_name("i")
    element.set_text(Lang.EN, "i")
    element.set_text(Lang.FA, "ایتالیک")

    element_dict = element.to_dict()

    base.print(element_dict)
    base.save(element_dict)
