import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

# Updated language code usage and to_string argument format
content_distributions = [
    Value().set_generate_name("space-between").add_text(Lang.languages["EN"]["code"], "space-between").add_text(Lang.languages["FA"]["code"], "فضا-بین"),
    Value().set_generate_name("space-around").add_text(Lang.languages["EN"]["code"], "space-around").add_text(Lang.languages["FA"]["code"], "فضا-دور"),
    Value().set_generate_name("space-evenly").add_text(Lang.languages["EN"]["code"], "space-evenly").add_text(Lang.languages["FA"]["code"], "فضا-یکسان"),
    Value().set_generate_name("stretch").add_text(Lang.languages["EN"]["code"], "stretch").add_text(Lang.languages["FA"]["code"], "کشیدن"),
]

if __name__ == "__main__":
    # Creating a dictionary with generate_name as key and value.to_dict() as value
    content_distributions_dict = {
        value.generate_name: value.to_dict() for value in content_distributions if value.generate_name is not None
    }

    # Generating the string output
    strings = Prebuild.to_string(content_distributions_dict)
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)


# https://developer.mozilla.org/en-US/docs/Web/CSS/align-content
# <content-distribution> =
#   space-between  |
#   space-around   |
#   space-evenly   |
#   stretch
