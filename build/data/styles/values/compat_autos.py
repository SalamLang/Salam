import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

# Updated language code usage and to_string argument format
compat_autos = [
    Value().set_generate_name("searchfield").add_text(Lang.languages["EN"]["code"], "searchfield").add_text(Lang.languages["FA"]["code"], "فیلد جستجو"),
    Value().set_generate_name("textarea").add_text(Lang.languages["EN"]["code"], "textarea").add_text(Lang.languages["FA"]["code"], "ناحیه متنی"),
    Value().set_generate_name("checkbox").add_text(Lang.languages["EN"]["code"], "checkbox").add_text(Lang.languages["FA"]["code"], "چک باکس"),
    Value().set_generate_name("radio").add_text(Lang.languages["EN"]["code"], "radio").add_text(Lang.languages["FA"]["code"], "رادیو"),
    Value().set_generate_name("menulist").add_text(Lang.languages["EN"]["code"], "menulist").add_text(Lang.languages["FA"]["code"], "لیست منو"),
    Value().set_generate_name("listbox").add_text(Lang.languages["EN"]["code"], "listbox").add_text(Lang.languages["FA"]["code"], "جعبه لیست"),
    Value().set_generate_name("meter").add_text(Lang.languages["EN"]["code"], "meter").add_text(Lang.languages["FA"]["code"], "متر"),
    Value().set_generate_name("progress-bar").add_text(Lang.languages["EN"]["code"], "progress-bar").add_text(Lang.languages["FA"]["code"], "نوار پیشرفت"),
    Value().set_generate_name("button").add_text(Lang.languages["EN"]["code"], "button").add_text(Lang.languages["FA"]["code"], "دکمه"),
]

if __name__ == "__main__":
    # Creating a dictionary with generate_name as key and value.to_dict() as value
    compat_autos_dict = {
        value.generate_name: value.to_dict() for value in compat_autos if value.generate_name is not None
    }

    # Generating the string output
    strings = Prebuild.to_string(compat_autos_dict)
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/appearance
# <compat-auto> = 
#   searchfield   |
#   textarea      |
#   checkbox      |
#   radio         |
#   menulist      |
#   listbox       |
#   meter         |
#   progress-bar  |
#   button        
