import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

crossorigins = [
    Value().set_generate_name("anonymous").add_text(Lang.EN, "anonymous").add_text(Lang.FA, "ناشناس"),
    Value().set_generate_name("use-credentials").add_text(Lang.EN, "use credentials").add_text(Lang.FA, "یوز کرید"),
]

if __name__ == '__main__':
    strings = Prebuild.to_string(
        [value.to_dict() for value in crossorigins]
    )
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)
