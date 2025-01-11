import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

compat_autos = [
    Value().set_generate_name("searchfield").add_text(Lang.EN, "searchfield").add_text(Lang.FA, "فیلد جستجو"),
    Value().set_generate_name("textarea").add_text(Lang.EN, "textarea").add_text(Lang.FA, "ناحیه متنی"),
    Value().set_generate_name("checkbox").add_text(Lang.EN, "checkbox").add_text(Lang.FA, "چک باکس"),
    Value().set_generate_name("radio").add_text(Lang.EN, "radio").add_text(Lang.FA, "رادیو"),
    Value().set_generate_name("menulist").add_text(Lang.EN, "menulist").add_text(Lang.FA, "لیست منو"),
    Value().set_generate_name("listbox").add_text(Lang.EN, "listbox").add_text(Lang.FA, "جعبه لیست"),
    Value().set_generate_name("meter").add_text(Lang.EN, "meter").add_text(Lang.FA, "متر"),
    Value().set_generate_name("progress-bar").add_text(Lang.EN, "progress-bar").add_text(Lang.FA, "نوار پیشرفت"),
    Value().set_generate_name("button").add_text(Lang.EN, "button").add_text(Lang.FA, "دکمه"),
]

if __name__ == "__main__":
    strings = Prebuild.to_string([value.to_dict() for value in compat_autos])
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
