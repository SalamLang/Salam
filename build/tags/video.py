import os
import sys
from pathlib import Path

# Ensure paths are correctly set up
parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "tags"))
sys.path.append(os.path.join(parent_dir, "data", "tags", "values"))

# Import necessary modules
from prebuild import Prebuild
from type import Type  # Ensure Type is imported from the correct file
from lang import Lang
from value import Value
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("video")

    # Add translations using Lang class
    element.add_text(Lang.languages["EN"]["code"], "video")
    element.add_text(Lang.languages["FA"]["code"], "فیلم")

    # Add attributes with correct Type constants
    element.add_attribute().set_generate_name("autoplay").set_type(
        Type.Boolean  # استفاده از Type صحیح از همان فایل
    ).add_text(Lang.languages["EN"]["code"], "autoplay").add_text(
        Lang.languages["FA"]["code"], "پخش خودکار"
    )

    element.add_attribute().set_generate_name("controls").set_type(
        Type.Boolean
    ).add_text(Lang.languages["EN"]["code"], "controls").add_text(
        Lang.languages["FA"]["code"], "کنترلر"
    )

    element.add_attribute().set_generate_name("height").set_type(Type.Size).add_text(
        Lang.languages["EN"]["code"], "height"
    ).add_text(Lang.languages["FA"]["code"], "ارتفاع")

    element.add_attribute().set_generate_name("loop").set_type(Type.Boolean).add_text(
        Lang.languages["EN"]["code"], "loop"
    ).add_text(Lang.languages["FA"]["code"], "تکرار")

    element.add_attribute().set_generate_name("muted").set_type(Type.Boolean).add_text(
        Lang.languages["EN"]["code"], "muted"
    ).add_text(Lang.languages["FA"]["code"], "بی صدا")

    element.add_attribute().set_generate_name("poster").set_type(Type.Url).add_text(
        Lang.languages["EN"]["code"], "poster"
    ).add_text(Lang.languages["FA"]["code"], "پوستر")

    element.add_attribute().set_generate_name("preload").set_type(Type.Url).add_text(
        Lang.languages["EN"]["code"], "preload"
    ).add_text(Lang.languages["FA"]["code"], "پیش لود").add_reserve_values(
        [
            Value()
            .set_generate_name("auto")
            .add_text(Lang.languages["EN"]["code"], "auto")
            .add_text(Lang.languages["FA"]["code"], "خودکار"),
            Value()
            .set_generate_name("metadata")
            .add_text(Lang.languages["EN"]["code"], "metadata")
            .add_text(Lang.languages["FA"]["code"], "متادیتا"),
            Value()
            .set_generate_name("none")
            .add_text(Lang.languages["EN"]["code"], "none")
            .add_text(Lang.languages["FA"]["code"], "هیچکدام"),
        ]
    )

    element.add_attribute().set_generate_name("src").set_type(Type.Url).add_text(
        Lang.languages["EN"]["code"], "source"
    ).add_text(Lang.languages["FA"]["code"], "منبع").set_is_required(True)

    element.add_attribute().set_generate_name("width").set_type(Type.Size).add_text(
        Lang.languages["EN"]["code"], "width"
    ).add_text(Lang.languages["FA"]["code"], "عرض")

    # Convert element to dictionary
    element_dict = element.to_dict()

    # Convert dictionary to string and save
    element_str = Prebuild.to_string(element_dict)
    Prebuild.print(element_str)
    Prebuild.save(element_str, __file__)
