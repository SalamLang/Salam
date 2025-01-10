import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "styles"))
sys.path.append(os.path.join(parent_dir, "data", "styles", "values"))

import self_positions
import overflow_positions
import baseline_positions
from lang import Lang
from type import Type
from value import Value
from values import Values
from prebuild import Prebuild
from property import Property

if __name__ == "__main__":
    property = Property()
    property.set_generate_name("align-items")
    property.add_text(Lang.EN, "align-items")
    property.add_text(Lang.FA, "الاین-آیتمز")
    property.set_type(Type.String)
    property.add_reserve_values(
        [
            Value()
            .set_generate_name("normal")
            .add_text(Lang.EN, "normal")
            .add_text(Lang.FA, "نرمال"),
            Value()
            .set_generate_name("stretch")
            .add_text(Lang.EN, "stretch")
            .add_text(Lang.FA, "کشیده"),
            Value()
            .set_generate_name("center")
            .add_text(Lang.EN, "center")
            .add_text(Lang.FA, "وسط"),
        ]
    )
    property.add_reserve_values(baseline_positions.baseline_positions)
    property.add_reserve_values(self_positions.self_positions)
    property.add_reserve_values(
        Values.And(
            overflow_positions.overflow_positions,
            self_positions.self_positions,
        )
    )

    property_dict = property.to_dict()

    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/align-items
# align-items =
#   normal                                    |
#   stretch                                   |
#   <baseline-position>                       |
#   [ <overflow-position>? <self-position> ]  |
#   anchor-center

# <baseline-position> =
#   [ first | last ]?  &&
#   baseline

# <overflow-position> =
#   unsafe  |
#   safe

# <self-position> =
#   center      |
#   start       |
#   end         |
#   self-start  |
#   self-end    |
#   flex-start  |
#   flex-end
