import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

compat_specials = [
    Value().set_generate_name("textfield").add_text(Lang.EN, "textfield").add_text(Lang.FA, "فیلد متنی"),
    Value().set_generate_name("menulist-button").add_text(Lang.EN, "menulist-button").add_text(Lang.FA, "دکمه لیست منو"),
]

if __name__ == "__main__":
    strings = Prebuild.to_string([value.to_dict() for value in compat_specials])
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/appearance
# <compat-special> = 
#   textfield        |
#   menulist-button  
