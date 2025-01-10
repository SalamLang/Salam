import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "styles"))
sys.path.append(os.path.join(parent_dir, "data", "styles", "values"))

from prebuild import Prebuild

# import single_animation_iteration_counts
from lang import Lang
from type import Type
from value import Value
from property import Property

if __name__ == "__main__":
    property = Property()
    property.set_generate_name("animation-iteration-count")
    property.add_text(Lang.EN, "animation-iteration-count")
    property.add_text(Lang.FA, "تعداد تکرار انیمیشن")
    property.set_type(Type.PositiveInt)
    property.add_reserve_values([
        Value().set_generate_name("infinite").add_text(Lang.EN, "infinite").add_text(Lang.FA, "بی‌پایان"),
    ])

    property_dict = property.to_dict()

    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/animation-iteration-count
# animation-iteration-count = 
#   <single-animation-iteration-count>#  

# <single-animation-iteration-count> = 
#   infinite        |
#   <number [0,∞]>  

