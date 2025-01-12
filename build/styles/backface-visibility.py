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
    property.set_generate_name("backface-visibility")
    property.add_text(Lang.EN, "backface-visibility")
    property.add_text(Lang.FA, "قابلیت دید پشتی")
    property.set_type(Type.String)

    property_dict = property.to_dict()

    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/block-size
# block-size = 
#   <'width'>  

# <width> = 
#   auto                                      |
#   <length-percentage [0,∞]>                 |
#   min-content                               |
#   max-content                               |
#   fit-content( <length-percentage [0,∞]> )  |
#   <calc-size()>                             |
#   <anchor-size()>                           

# <length-percentage> = 
#   <length>      |
#   <percentage>  

# <calc-size()> = 
#   calc-size( <calc-size-basis> , <calc-sum> )  

# <anchor-size()> = 
#   anchor-size( [ <anchor-name> || <anchor-size> ]? , <length-percentage>? )  

# <calc-size-basis> = 
#   <intrinsic-size-keyword>  |
#   <calc-size()>             |
#   any                       |
#   <calc-sum>                

# <calc-sum> = 
#   <calc-product> [ [ '+' | '-' ] <calc-product> ]*  

# <anchor-name> = 
#   <dashed-ident>  

# <anchor-size> = 
#   width        |
#   height       |
#   block        |
#   inline       |
#   self-block   |
#   self-inline  

# <calc-product> = 
#   <calc-value> [ [ '*' | '/' ] <calc-value> ]*  

# <calc-value> = 
#   <number>        |
#   <dimension>     |
#   <percentage>    |
#   <calc-keyword>  |
#   ( <calc-sum> )  

# <calc-keyword> = 
#   e          |
#   pi         |
#   infinity   |
#   -infinity  |
#   NaN        
