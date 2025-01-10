import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

self_positions = [
    Value().set_generate_name("center").add_text(Lang.EN, "center").add_text(Lang.FA, "وسط"),
    Value().set_generate_name("start").add_text(Lang.EN, "start").add_text(Lang.FA, "شروع"),
    Value().set_generate_name("end").add_text(Lang.EN, "end").add_text(Lang.FA, "پایان"),
    Value().set_generate_name("self-start").add_text(Lang.EN, "self-start").add_text(Lang.FA, "شروع خود"),
    Value().set_generate_name("self-end").add_text(Lang.EN, "self-end").add_text(Lang.FA, "پایان خود"),
    Value().set_generate_name("flex-start").add_text(Lang.EN, "flex-start").add_text(Lang.FA, "شروع فلکس"),
    Value().set_generate_name("flex-end").add_text(Lang.EN, "flex-end").add_text(Lang.FA, "پایان فلکس"),
]

if __name__ == '__main__':
    strings = Prebuild.to_string(
        [value.to_dict() for value in self_positions]
    )
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/align-items
# <self-position> = 
#   center      |
#   start       |
#   end         |
#   self-start  |
#   self-end    |
#   flex-start  |
#   flex-end