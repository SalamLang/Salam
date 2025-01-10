import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent
sys.path.append(str(parent_dir))

from value import Value
from prebuild import Prebuild

single_animation_play_states = [
    Value().set_generate_name("running").add_text("en", "running").add_text("fa", "در حال اجرا"),
    Value().set_generate_name("paused").add_text("en", "paused").add_text("fa", "متوقف"),
]

if __name__ == '__main__':
    strings = Prebuild.to_string(
        [value.to_dict() for value in single_animation_play_states]
    )
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/animation-play-state
# <single-animation-play-state> = 
#   running  |
#   paused   
