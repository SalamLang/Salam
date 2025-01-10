from lang import Lang
from value import Value

overflow_positions = [
    Value().set_generate_name("first baseline").add_text(Lang.EN, "first baseline").add_text(Lang.FA, "خط پایه اول"),
    Value().set_generate_name("first baseline").add_text(Lang.EN, "baseline first").add_text(Lang.FA, "اول خط پایه"),
    Value().set_generate_name("last baseline").add_text(Lang.EN, "last baseline").add_text(Lang.FA, "خط پایه آخر"),
    Value().set_generate_name("last baseline").add_text(Lang.EN, "baseline last").add_text(Lang.FA, "آخر خط پایه"),
    Value().set_generate_name("baseline").add_text(Lang.EN, "baseline").add_text(Lang.FA, "خط پایه"),

]

# https://developer.mozilla.org/en-US/docs/Web/CSS/align-items
# <baseline-position> = 
#   [ first | last ]?  &&
#   baseline           
