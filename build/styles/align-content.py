import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "styles"))
sys.path.append(os.path.join(parent_dir, "data", "styles", "values"))

import content_distributions
import overflow_positions
import baseline_positions
import content_positions
from lang import Lang
from type import Type
from value import Value
from values import Values
from prebuild import Prebuild
from property import Property, PropertyType

if __name__ == "__main__":
    property = Property()
    property.set_generate_name("align-content")
    property.add_text(Lang.languages["EN"]["code"], "align-content")
    property.add_text(Lang.languages["FA"]["code"], "تراز محتوا")

    # Use PropertyType.String instead of Type.String
    property.set_type(PropertyType.String)

    property.add_reserve_values(
        [
            Value()
            .set_generate_name("normal")
            .add_text(Lang.languages["EN"]["code"], "normal")
            .add_text(Lang.languages["FA"]["code"], "نرمال"),
        ]
    )

    property.add_reserve_values(
        list(baseline_positions.baseline_positions)
    )  # Convert to list
    property.add_reserve_values(
        list(content_distributions.content_distributions)
    )  # Convert to list

    property.add_reserve_values(
        Values.AndOrder(
            tuple(overflow_positions.overflow_positions),  # Convert to tuple
            tuple(content_positions.content_positions),  # Convert to tuple
        )
    )

    property.add_reserve_values(
        list(content_positions.content_positions)
    )  # Convert to list

    property_dict = property.to_dict()

    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/align-content
# align-content =
#   normal                                   |
#   <baseline-position>                      |
#   <content-distribution>                   |
#   <overflow-position>? <content-position>

# <baseline-position> =
#   [ first | last ]?  &&
#   baseline

# <content-distribution> =
#   space-between  |
#   space-around   |
#   space-evenly   |
#   stretch

# <overflow-position> =
#   unsafe  |
#   safe

# <content-position> =
#   center      |
#   start       |
#   end         |
#   flex-start  |
#   flex-end
