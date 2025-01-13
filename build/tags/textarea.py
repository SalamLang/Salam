import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "tags"))
sys.path.append(os.path.join(parent_dir, "data", "tags", "values"))

from prebuild import Prebuild
from type import Type
from lang import Lang
from value import Value
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("textarea")
    element.add_text(Lang.languages["EN"]["code"], "textarea")
    element.add_text(Lang.languages["FA"]["code"], "ویرایشگر")

    element.add_attribute().set_generate_name("autofocus").set_type(
        Type.Boolean
    ).add_text(Lang.languages["EN"]["code"], "autofocus").add_text(Lang.languages["FA"]["code"], "فوکوس خودکار")
    element.add_attribute().set_generate_name("cols").set_type(Type.Number).add_text(
        Lang.languages["EN"]["code"], "cols"
    ).add_text(Lang.languages["FA"]["code"], "کول")
    # TODO
    element.add_attribute().set_generate_name("dirname").set_type(
        Type.AnyNoEmpty
    ).add_text(Lang.languages["EN"]["code"], "dirname").add_text(Lang.languages["FA"]["code"], "نام جهت")
    element.add_attribute().set_generate_name("disabled").set_type(
        Type.Boolean
    ).add_text(Lang.languages["EN"]["code"], "disabled").add_text(Lang.languages["FA"]["code"], "غیرفعال")
    element.add_attribute().set_generate_name("form").set_type(Type.ID).add_text(
        Lang.languages["EN"]["code"], "form"
    ).add_text(Lang.languages["FA"]["code"], "فرم")
    element.add_attribute().set_generate_name("max_length").set_type(
        Type.PositiveInt
    ).add_text(Lang.languages["EN"]["code"], "max length").add_text(Lang.languages["FA"]["code"], "حداکثر طول")
    element.add_attribute().set_generate_name("name").set_type(
        Type.AnyNoEmpty
    ).add_text(Lang.languages["EN"]["code"], "name").add_text(Lang.languages["FA"]["code"], "نام")
    element.add_attribute().set_generate_name("placeholder").set_type(
        Type.String
    ).add_text(Lang.languages["EN"]["code"], "placeholder").add_text(Lang.languages["EN"]["code"], "راهنما")
    element.add_attribute().set_generate_name("readonly").set_type(
        Type.Boolean
    ).add_text(Lang.languages["EN"]["code"], "readonly").add_text(Lang.languages["FA"]["code"], "فقط خواندنی")
    element.add_attribute().set_generate_name("required").set_type(
        Type.Boolean
    ).add_text(Lang.languages["EN"]["code"], "required").add_text(Lang.languages["FA"]["code"], "اجباری")
    element.add_attribute().set_generate_name("rows").set_type(Type.Boolean).add_text(
        Lang.languages["EN"]["code"], "rows"
    ).add_text(Lang.languages["FA"]["code"], "ردیف")
    element.add_attribute().set_generate_name("wrap").set_type(Type.String).add_text(
        Lang.languages["EN"]["code"], "wrap"
    ).add_text(Lang.languages["FA"]["code"], "رپ").add_reserve_values(
        [
            Value()
            .set_generate_name("soft")
            .add_text(Lang.languages["EN"]["code"], "soft")
            .add_text(Lang.languages["FA"]["code"], "نرم"),
            Value()
            .set_generate_name("hard")
            .add_text(Lang.languages["EN"]["code"], "hard")
            .add_text(Lang.languages["FA"]["code"], "سخت"),
        ]
    )
    element_dict = element.to_dict()

    element_str = Prebuild.to_string(element_dict)
    Prebuild.print(element_str)
    Prebuild.save(element_str, __file__)
