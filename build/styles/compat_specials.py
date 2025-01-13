import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang  # Assuming this is where Lang is defined as a class
from value import Value
from prebuild import Prebuild

compat_specials = [
    Value().set_generate_name("textfield").add_text(Lang.languages["EN"]["code"], "textfield").add_text(Lang.languages["FA"]["code"], "فیلد متنی"),
    Value().set_generate_name("menulist-button").add_text(Lang.languages["EN"]["code"], "menulist-button").add_text(Lang.languages["FA"]["code"], "دکمه لیست منو"),
]

if __name__ == "__main__":
    # Ensure Prebuild.to_string() gets a dictionary
    strings = Prebuild.to_string({value.generate_name or "default_key": value.to_dict() for value in compat_specials})
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/appearance
# <compat-special> = 
#   textfield        |
#   menulist-button  
