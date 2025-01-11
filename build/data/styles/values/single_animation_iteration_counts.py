import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

from prebuild import Prebuild

single_animation_iteration_counts = [
    # TODO
]

if __name__ == "__main__":
    strings = Prebuild.to_string(
        [value.to_dict() for value in single_animation_iteration_counts]
    )
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/animation-iteration-count
# <single-animation-iteration-count> =
#   infinite        |
#   <number [0,∞]>
