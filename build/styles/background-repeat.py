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
from value import Value
from values import Values
from property import Property

if __name__ == "__main__":
    property = Property()
    property.set_generate_name("background-repeat")
    property.add_text(Lang.EN, "background-repeat")
    property.add_text(Lang.FA, "تکرار پس زمینه")
    property.set_type(Type.SIZE)

    values = [
        Value().set_generate_name("repeat").add_text(Lang.EN, "repeat").add_text(Lang.FA, "تکرار"),
        Value().set_generate_name("space").add_text(Lang.EN, "space").add_text(Lang.FA, "فضا"),
        Value().set_generate_name("round").add_text(Lang.EN, "round").add_text(Lang.FA, "گرد"),
        Value().set_generate_name("no-repeat").add_text(Lang.EN, "no-repeat").add_text(Lang.FA, "بدون تکرار"),
    ]
    property.add_reserve_values(
        [
            Value().set_generate_name("repeat-x").add_text(Lang.EN, "repeat-x").add_text(Lang.FA, "تکرار افقی"),
            Value().set_generate_name("repeat-y").add_text(Lang.EN, "repeat-y").add_text(Lang.FA, "تکرار عمودی"),
        ]
    )
    property.add_values(values)
    property.add_reserve_values(Values.And(values, values))

    property_dict = property.to_dict()

    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/background-repeat
# background-repeat = 
#   <repeat-style>#  

# <repeat-style> = 
#   repeat-x                                     |
#   repeat-y                                     |
#   [ repeat | space | round | no-repeat ]{1,2}  
