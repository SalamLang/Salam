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
from property import Property, PropertyType

if __name__ == "__main__":
    property = Property()

    property.set_generate_name("align-self")

    property.add_text(Lang.languages["EN"]["code"], "align-self")
    property.add_text(Lang.languages["FA"]["code"], "تراز خود")

    property.set_type(PropertyType.String)

    property.add_reserve_values(
        [
            Value()
            .set_generate_name("auto")
            .add_text(Lang.languages["EN"]["code"], "auto")
            .add_text(Lang.languages["FA"]["code"], "اتوماتیک"),
            Value()
            .set_generate_name("normal")
            .add_text(Lang.languages["EN"]["code"], "normal")
            .add_text(Lang.languages["FA"]["code"], "نرمال"),
            Value()
            .set_generate_name("stretch")
            .add_text(Lang.languages["EN"]["code"], "stretch")
            .add_text(Lang.languages["FA"]["code"], "کشیده"),
            Value()
            .set_generate_name("anchor-center")
            .add_text(Lang.languages["EN"]["code"], "anchor-center")
            .add_text(Lang.languages["FA"]["code"], "مرکز قایق"),
        ]
    )

    property.add_reserve_values(baseline_positions.baseline_positions)

    # Convert tuple to list for add_reserve_values
    property.add_reserve_values(
        list(
            Values.AndOrder(  # Convert to list
                tuple(overflow_positions.overflow_positions),
                tuple(self_positions.self_positions),
            )
        )
    )

    # Convert tuple to list for add_reserve_values
    property.add_reserve_values(
        list(tuple(self_positions.self_positions))
    )  # Convert to list

    property_dict = property.to_dict()

    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/align-self
# align-self =
#   auto                                  |
#   normal                                |
#   stretch                               |
#   <baseline-position>                   |
#   <overflow-position>? <self-position>  |
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
