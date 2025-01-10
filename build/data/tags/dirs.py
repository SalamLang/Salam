import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

dirs = [
    Value().set_generate_name("ltr").add_text(Lang.EN, "ltr").add_text(Lang.EN, "left").add_text(Lang.FA, "چپ به راست").add_text(Lang.FA, "چپ"),
    Value().set_generate_name("rtl").add_text(Lang.EN, "rtl").add_text(Lang.EN, "right").add_text(Lang.FA, "راست به چپ").add_text(Lang.FA, "راست"),
]

if __name__ == '__main__':
    strings = Prebuild.to_string(
        [value.to_dict() for value in dirs]
    )
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)
