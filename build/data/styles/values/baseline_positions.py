import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from values import Values
from prebuild import Prebuild

# Convert the lists into tuples
baseline_positions = Values.AndOrder(
    tuple(
        [
            Value()
            .set_generate_name("first")
            .add_text(Lang.languages["EN"]["code"], "first")
            .add_text(Lang.languages["FA"]["code"], "اول"),
            Value()
            .set_generate_name("last")
            .add_text(Lang.languages["EN"]["code"], "last")
            .add_text(Lang.languages["FA"]["code"], "آخر"),
        ]
    ),
    tuple(
        [
            Value()
            .set_generate_name("baseline")
            .add_text(Lang.languages["EN"]["code"], "baseline")
            .add_text(Lang.languages["FA"]["code"], "خط پایه"),
        ]
    ),
)

baseline_positions += [
    Value()
    .set_generate_name("baseline")
    .add_text(Lang.languages["EN"]["code"], "baseline")
    .add_text(Lang.languages["FA"]["code"], "خط پایه"),
]

if __name__ == "__main__":
    strings = Prebuild.to_string(
        {"baseline_positions": [value.to_dict() for value in baseline_positions]}
    )
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/align-items
# <baseline-position> =
#   [ first | last ]?  &&
#   baseline
