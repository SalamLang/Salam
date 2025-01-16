import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "styles"))
sys.path.append(os.path.join(parent_dir, "data", "styles", "values"))

from lang import Lang
from type import Type
from property import Property

if __name__ == "__main__":
    property = Property()
    property.set_generate_name("background-position-y")
    property.add_text(Lang.EN, "background-position-y")
    property.add_text(Lang.FA, 
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

# https://developer.mozilla.org/en-US/docs/Web/CSS/background-position-y
# background-position-y = 
#   [ center | [ [ top | bottom | y-start | y-end ]? <length-percentage>? ]! ]#  

# <length-percentage> = 
#   <length>      |
#   <percentage>  
