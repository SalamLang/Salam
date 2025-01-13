import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

# Corrected access to language codes and Prebuild.to_string() argument type
single_animation_play_states = [
    Value()
    .set_generate_name("running")
    .add_text(Lang.languages["EN"]["code"], "running")
    .add_text(Lang.languages["FA"]["code"], "در حال اجرا"),
    Value()
    .set_generate_name("paused")
    .add_text(Lang.languages["EN"]["code"], "paused")
    .add_text(Lang.languages["FA"]["code"], "متوقف"),
]

if __name__ == "__main__":
    # Creating a dictionary with generate_name as key and value.to_dict() as value
    single_animation_play_states_dict = {
        value.generate_name: value.to_dict() for value in single_animation_play_states if value.generate_name is not None
    }

    # Generating the string output
    strings = Prebuild.to_string(single_animation_play_states_dict)
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)


# https://developer.mozilla.org/en-US/docs/Web/CSS/animation-play-state
# <single-animation-play-state> =
#   running  |
#   paused
