import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "styles"))
sys.path.append(os.path.join(parent_dir, "data", "styles", "values"))

from prebuild import Prebuild

import easing_functions
from lang import Lang
from type import Type  # Assuming PropertyType is what is expected in set_type()
from property import Property, PropertyType  # Assuming PropertyType is imported here

if __name__ == "__main__":
    property = Property()
    property.set_generate_name("animation-timing-function")
    property.add_text(Lang.languages["EN"]["code"], "animation-timing-function")  # Adjusted for Lang dictionary
    property.add_text(Lang.languages["FA"]["code"], "تابع زمان انیمیشن")  # Adjusted for Lang dictionary
    property.set_type(PropertyType.String)  # Adjusted to use PropertyType instead of Type

    property.add_reserve_values(easing_functions.easing_functions)

    property_dict = property.to_dict()

    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/animation-timing-function
# animation-timing-function = 
#   <easing-function>#  

# <easing-function> = 
#   <linear-easing-function>        |
#   <cubic-bezier-easing-function>  |
#   <step-easing-function>          

# <linear-easing-function> = 
#   linear      |
#   <linear()>  

# <cubic-bezier-easing-function> = 
#   ease              |
#   ease-in           |
#   ease-out          |
#   ease-in-out       |
#   <cubic-bezier()>  

# <step-easing-function> = 
#   step-start  |
#   step-end    |
#   <steps()>   

# <linear()> = 
#   linear( [ <number> && <percentage>{0,2} ]# )  

# <cubic-bezier()> = 
#   cubic-bezier( [ <number [0,1]> , <number> ]#{2} )  

# <steps()> = 
#   steps( <integer> , <step-position>? )  

# <step-position> = 
#   jump-start  |
#   jump-end    |
#   jump-none   |
#   jump-both   |
#   start       |
#   end
