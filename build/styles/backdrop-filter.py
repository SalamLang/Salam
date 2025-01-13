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
from property import Property, PropertyType

# Mapping function for Type to PropertyType
def type_to_property_type(type_: Type) -> PropertyType:
    type_mapping = {
        Type.NUMBER_OR_2NUMBERS: PropertyType.NUMBER_OR_2NUMBERS,  # Adjust mapping here
        # Add other mappings as necessary
    }
    return type_mapping.get(type_, PropertyType.String)  # Default to STRING if no mapping found


if __name__ == "__main__":
    property = Property()
    property.set_generate_name("backdrop-filter")
    property.add_text(Lang.languages["EN"]["code"], "backdrop-filter")
    property.add_text(Lang.languages["FA"]["code"], "فیلتر پس‌زمینه")
    property.set_type(type_to_property_type(Type.NUMBER_OR_2NUMBERS))
    property.add_reserve_values(
        [
            Value().set_generate_name("auto").add_text(Lang.languages["EN"]["code"], "auto").add_text(Lang.languages["FA"]["code"], "خودکار"),
        ]
    )

    property_dict = property.to_dict()

    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/backdrop-filter
# backdrop-filter = 
#   none                 |
#   <filter-value-list>  

# <filter-value-list> = 
#   [ <filter-function> | <url> ]+  

# <filter-function> = 
#   <blur()>         |
#   <brightness()>   |
#   <contrast()>     |
#   <drop-shadow()>  |
#   <grayscale()>    |
#   <hue-rotate()>   |
#   <invert()>       |
#   <opacity()>      |
#   <sepia()>        |
#   <saturate()>     

# <url> = 
#   <url()>  |
#   <src()>  

# <blur()> = 
#   blur( <length>? )  

# <brightness()> = 
#   brightness( [ <number> | <percentage> ]? )  

# <contrast()> = 
#   contrast( [ <number> | <percentage> ]? )  

# <drop-shadow()> = 
#   drop-shadow( [ <color>? && <length>{2,3} ] )  

# <grayscale()> = 
#   grayscale( [ <number> | <percentage> ]? )  

# <hue-rotate()> = 
#   hue-rotate( [ <angle> | <zero> ]? )  

# <invert()> = 
#   invert( [ <number> | <percentage> ]? )  

# <opacity()> = 
#   opacity( [ <number> | <percentage> ]? )  

# <sepia()> = 
#   sepia( [ <number> | <percentage> ]? )  

# <saturate()> = 
#   saturate( [ <number> | <percentage> ]? )  

# <url()> = 
#   url( <string> <url-modifier>* )  |
#   <url-token>                      

# <src()> = 
#   src( <string> <url-modifier>* )  
