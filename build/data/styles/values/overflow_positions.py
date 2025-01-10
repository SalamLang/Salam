import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

overflow_positions = [
    Value().set_generate_name("safe").add_text(Lang.EN, "safe").add_text(Lang.FA, "امن"),
    Value().set_generate_name("unsafe").add_text(Lang.EN, "unsafe").add_text(Lang.FA, "نامناسب"),
]

if __name__ == '__main__':
    strings = Prebuild.to_string(
        [value.to_dict() for value in overflow_positions]
    )
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/align-items
# <overflow-position> = 
#   unsafe  |
#   safe    
