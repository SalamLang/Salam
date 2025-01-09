import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

import base
from lang import Lang
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("noscript")
    element.set_text(Lang.EN, "noscript")
    element.set_text(Lang.FA, "بدون اسکریپت")

    element_dict = element.to_dict()

    base.print(element_dict)
    base.save(element_dict)
