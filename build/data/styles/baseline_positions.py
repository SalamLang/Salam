import itertools
import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir))

from lang import Lang
from value import Value
from values import Values

overflow_positions = Values.And(
    [
        Value().set_generate_name("first").add_text(Lang.EN, "first").add_text(Lang.FA, "اول"),
        Value().set_generate_name("last").add_text(Lang.EN, "last").add_text(Lang.FA, "آخر"),
    ],
    [
        Value().set_generate_name("baseline").add_text(Lang.EN, "baseline").add_text(Lang.FA, "خط پایه"),
    ],
)
overflow_positions += [
    Value().set_generate_name("baseline").add_text(Lang.EN, "baseline").add_text(Lang.FA, "خط پایه"),
]

print(overflow_positions)

# https://developer.mozilla.org/en-US/docs/Web/CSS/align-items
# <baseline-position> = 
#   [ first | last ]?  &&
#   baseline           


# data = {
#     'EN': [['baseline'], ['last']],
#     'FA': [['خط پایه'], ['آخر']]
# }

# def generate_ordered_strings(data):
#     result = {}
#     for lang, text_groups in data.items():
#         combinations = itertools.product(*text_groups)
#         result[lang] = [' '.join(' '.join(item) for item in combo) for combo in combinations]
#     return result

# output = generate_ordered_strings(data)

# # Print the results
# for lang, strings in output.items():
#     print(f"{lang}: {strings}")
