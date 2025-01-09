import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

from build import Build
from type import Type
from lang import Lang
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("select")
    element.set_text(Lang.EN, "select")
    element.set_text(Lang.FA, "فهرست کشویی")

    element.add_attribute() \
        .set_generate_name("autofocus") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "autofocus") \
        .add_text(Lang.FA, "فوکوس خودکار") \

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
        .set_generate_name("multiple") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "multiple") \
        .add_text(Lang.FA, "چندتایی") \

    element.add_attribute() \
        .set_generate_name("name") \
        .set_type(Type.AnyNoEmpty) \
        .add_text(Lang.EN, "name") \
        .add_text(Lang.FA, "نام") \

    element.add_attribute() \
        .set_generate_name("required") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "required") \
        .add_text(Lang.FA, "اجباری") \

    element.add_attribute() \
        .set_generate_name("size") \
        .set_type(Type.PositiveInt) \
        .add_text(Lang.EN, "size") \
        .add_text(Lang.FA, "تعداد") \

    element_dict = element.to_dict()

    element_str = Build.to_string(element_dict)
    Build.print(element_str)
    Build.save(element_str, __file__)
