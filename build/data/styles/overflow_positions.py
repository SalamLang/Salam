from lang import Lang
from value import Value

overflow_positions = [
    Value().set_generate_name("safe").add_text(Lang.EN, "safe").add_text(Lang.FA, "امن"),
    Value().set_generate_name("unsafe").add_text(Lang.EN, "unsafe").add_text(Lang.FA, "نامناسب"),
]

# https://developer.mozilla.org/en-US/docs/Web/CSS/align-items
# <overflow-position> = 
#   unsafe  |
#   safe    
