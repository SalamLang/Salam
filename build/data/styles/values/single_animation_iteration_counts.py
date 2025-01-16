import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

from prebuild import Prebuild

# Create a dictionary with the animation iteration counts as a list of dictionaries
single_animation_iteration_counts: dict[str, list[dict[str, str]]] = {
    "iteration_counts": [
        {"count": "infinite"},
        {"count": "3"},
    ]
}

if __name__ == "__main__":
    # Now passing a single dictionary as expected
    strings = Prebuild.to_string(single_animation_iteration_counts)
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/animation-iteration-count
# <single-animation-iteration-count> =
#   infinite        |
#   <number [0,∞]>
