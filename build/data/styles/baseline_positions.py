import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir))

from lang import Lang
from value import Value
from values import Values

overflow_positions = Values.And(
    [
        Value().set_generate_name("first").add_text(Lang.EN, "first").add_text(Lang.FA, "اول"),
        Value().set_generate_name("last").add_text(Lang.EN, "last").add_text(Lang.FA, "آخر"),
    ],
    [
        Value().set_generate_name("baseline").add_text(Lang.EN, "baseline").add_text(Lang.FA, "خط پایه"),
    ],
)
# Values.and() should generate the following:
# Value().set_generate_name("first baseline").add_text(Lang.EN, "first baseline").add_text(Lang.FA, "خط پایه اول"),
# Value().set_generate_name("first baseline").add_text(Lang.EN, "baseline first").add_text(Lang.FA, "اول خط پایه"),
# Value().set_generate_name("last baseline").add_text(Lang.EN, "last baseline").add_text(Lang.FA, "خط پایه آخر"),
# Value().set_generate_name("last baseline").add_text(Lang.EN, "baseline last").add_text(Lang.FA, "آخر خط پایه"),
# Value().set_generate_name("baseline").add_text(Lang.EN, "baseline").add_text(Lang.FA, "خط پایه"),

print(overflow_positions)


overflow_positions += [
    Value().set_generate_name("baseline").add_text(Lang.EN, "baseline").add_text(Lang.FA, "خط پایه"),
]

print(overflow_positions)

# https://developer.mozilla.org/en-US/docs/Web/CSS/align-items
# <baseline-position> = 
#   [ first | last ]?  &&
#   baseline           
