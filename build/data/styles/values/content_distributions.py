import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

content_distributions = [
    Value()
    .set_generate_name("space-between")
    .add_text(Lang.EN, "space-between")
    .add_text(Lang.FA, "فضا-بین"),
    Value()
    .set_generate_name("space-around")
    .add_text(Lang.EN, "space-around")
    .add_text(Lang.FA, "فضا-دور"),
    Value()
    .set_generate_name("space-evenly")
    .add_text(Lang.EN, "space-evenly")
    .add_text(Lang.FA, "فضا-یکسان"),
    Value()
    .set_generate_name("stretch")
    .add_text(Lang.EN, "stretch")
    .add_text(Lang.FA, "کشیدن"),
]

if __name__ == "__main__":
    strings = Prebuild.to_string([value.to_dict() for value in content_distributions])
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/align-content
# <content-distribution> =
#   space-between  |
#   space-around   |
#   space-evenly   |
#   stretch
