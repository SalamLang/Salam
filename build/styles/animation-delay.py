import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "styles"))
sys.path.append(os.path.join(parent_dir, "data", "styles", "values"))

from prebuild import Prebuild
from lang import Lang
from property import Property, PropertyType
from value import Value

if __name__ == "__main__":
    property = Property()
    property.set_generate_name("animation-delay")
    property.add_text(Lang.languages["EN"]["code"], "animation-delay")
    property.add_text(Lang.languages["FA"]["code"], "تاخیر انیمیشن")
    property.set_type(PropertyType.Times)
    property_dict = property.to_dict()

    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/animation-delay
# animation-delay =
#   <time>#
