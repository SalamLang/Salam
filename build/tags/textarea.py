import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

import build
from type import Type
from lang import Lang
from value import Value
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("textarea")
    element.set_text(Lang.EN, "textarea")
    element.set_text(Lang.FA, "ویرایشگر")

    element.add_attribute() \
        .set_generate_name("autofocus") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "autofocus") \
        .add_text(Lang.FA, "فوکوس خودکار") \

    element.add_attribute() \
        .set_generate_name("cols") \
        .set_type(Type.Number) \
        .add_text(Lang.EN, "cols") \
        .add_text(Lang.FA, "کول") \

    # TODO
    element.add_attribute() \
        .set_generate_name("dirname") \
        .set_type(Type.AnyNoEmpty) \
        .add_text(Lang.EN, "dirname") \
        .add_text(Lang.FA, "نام جهت") \

    element.add_attribute() \
        .set_generate_name("disabled") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "disabled") \
        .add_text(Lang.FA, "غیرفعال") \

    element.add_attribute() \
        .set_generate_name("form") \
        .set_type(Type.ID) \
        .add_text(Lang.EN, "form") \
        .add_text(Lang.FA, "فرم") \

    element.add_attribute() \
        .set_generate_name("max_length") \
        .set_type(Type.PositiveInt) \
        .add_text(Lang.EN, "max length") \
        .add_text(Lang.FA, "حداکثر طول") \

    element.add_attribute() \
        .set_generate_name("name") \
        .set_type(Type.AnyNoEmpty) \
        .add_text(Lang.EN, "name") \
        .add_text(Lang.FA, "نام") \

    element.add_attribute() \
        .set_generate_name("placeholder") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "placeholder") \
        .add_text(Lang.FA, "راهنما") \

    element.add_attribute() \
        .set_generate_name("readonly") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "readonly") \
        .add_text(Lang.FA, "فقط خواندنی") \

    element.add_attribute() \
        .set_generate_name("required") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "required") \
        .add_text(Lang.FA, "اجباری") \

    element.add_attribute() \
        .set_generate_name("rows") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "rows") \
        .add_text(Lang.FA, "ردیف") \

    element.add_attribute() \
        .set_generate_name("wrap") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "wrap") \
        .add_text(Lang.FA, "رپ") \
        .add_reserve_values([
            Value().set_generate_name("soft").add_text(Lang.EN, "soft").add_text(Lang.FA, "نرم"),
            Value().set_generate_name("hard").add_text(Lang.EN, "hard").add_text(Lang.FA, "سخت"),
        ]) \

    element_dict = element.to_dict()

    build.print(element_dict)
    build.save(element_dict)
