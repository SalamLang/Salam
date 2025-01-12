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
from property import Property

if __name__ == "__main__":
    property = Property()
    property.set_generate_name("block-size")
    property.add_text(Lang.EN, "block-size")
    property.add_text(Lang.FA, "اندازه بلوک")
    property.set_type(Type.NUMBER_OR_2NUMBERS)
    property.add_reserve_values(
        [
            Value().set_generate_name("visible").add_text(Lang.EN, "visible").add_text(Lang.FA, "قابل دید"),
            Value().set_generate_name("hidden").add_text(Lang.EN, "hidden").add_text(Lang.FA, "پنهان"),
        ]
    )

    property_dict = property.to_dict()

    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/backface-visibility
# backface-visibility = 
#   visible  |
#   hidden   
