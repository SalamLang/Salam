import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "styles"))
sys.path.append(os.path.join(parent_dir, "data", "styles", "values"))

from prebuild import Prebuild

from lang import Lang
from type import Type
from property import Property, PropertyType

if __name__ == "__main__":
    property = Property()
    property.set_generate_name("animation-duration")
    property.add_text(Lang.languages["EN"]["code"], "animation-duration")
    property.add_text(Lang.languages["FA"]["code"], "طول انیمیشن")
    property.set_type(PropertyType.Time)

    property_dict = property.to_dict()

    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/animation-duration
# animation-duration =
#   <time [0s,∞]>#a

# 12s         Positive integer
# -456ms      Negative integer
# 4.3ms       Non-integer
# 14mS        The unit is case-insensitive, although capital letters are not recommended.
# +0s         Zero with a leading + and a unit
# -0ms        Zero with a leading - and a unit
