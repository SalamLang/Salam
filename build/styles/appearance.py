import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "styles"))
sys.path.append(os.path.join(parent_dir, "data", "styles", "values"))

from prebuild import Prebuild

import compat_autos
import compat_specials
from lang import Lang
from type import Type
from value import Value
from property import Property

if __name__ == "__main__":
    property = Property()
    property.set_generate_name("appearance")
    property.add_text(Lang.EN, "appearance")
    property.add_text(Lang.FA, "ظاهر")
    property.set_type(Type.String)
    property.add_reserve_values(
        [
            Value().set_generate_name("none").add_text(Lang.EN, "none").add_text(Lang.FA, "هیچ"),
            Value().set_generate_name("auto").add_text(Lang.EN, "auto").add_text(Lang.FA, "خودکار"),
            Value().set_generate_name("base").add_text(Lang.EN, "base").add_text(Lang.FA, "پایه"),
        ]
    )
    property.add_reserve_values(compat_autos.compat_autos)
    property.add_reserve_values(compat_specials.compat_specials)

    property_dict = property.to_dict()

    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/appearance
# appearance = 
#   none              |
#   auto              |
#   base              |
#   <compat-auto>     |
#   <compat-special>  

# <compat-auto> = 
#   searchfield   |
#   textarea      |
#   checkbox      |
#   radio         |
#   menulist      |
#   listbox       |
#   meter         |
#   progress-bar  |
#   button        

# <compat-special> = 
#   textfield        |
#   menulist-button  
