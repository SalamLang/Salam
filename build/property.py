from text import Text
from prebuild import Prebuild
from attribute import Attribute
from type import Type
from value import Value

class Property:
    def __init__(self):
        self.text = Text()
        self.generate_name = None
        self.type = Type.String
        self.reserved_values = []

    def set_generate_name(self, generate_name):
        """Set the generate name for the main element."""
        self.generate_name = generate_name
        idtok = Prebuild.idize(generate_name.upper())
        self.id = "AST_LAYOUT_TYPE_" + idtok
        return self

    def set_text(self, language_code, text):
        """Set text for the main element."""
        self.text.add_translation(language_code, text)
        return self

    def set_type(self, type):
        """Set the type of the property."""
        self.type = type
        return self

    def add_reserve_value(self, value):
        """Add a reserved value for the property."""
        if not isinstance(value, Value):
            raise TypeError("Reserved value must be an instance of Value.")
        self.reserved_values.append(value)
        return self

    def add_reserve_values(self, values):
        """Add multiple reserved values for the property."""
        for value in values:
            self.add_reserve_value(value)
        return self

    def to_dict(self):
        """Convert the element to a dictionary, including all attributes and reserved values."""
        for value in self.reserved_values:
            value.validate()
        return {
            "id": self.id,
            "generate_name": self.generate_name,
            "type": self.type,
            "text": self.text.to_dict(),
            "reserved_values": [
                value.to_dict() for value in self.reserved_values
            ],
        }
