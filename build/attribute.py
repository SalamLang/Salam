from text import Text
from type import Type

class Attribute:
    def __init__(self):
        self.text = Text()
        self.type = Type.String
        self.reserved_values = []

    def set_generate_name(self, generate_name):
        """Set the generate name for the main element."""
        self.generate_name = generate_name
        self.id = "AST_LAYOUT_ATTRIBUTE_TYPE_" + generate_name.upper().replace(" ", "_").replace("-", "_").replace("__", "_")
        return self

    def set_type(self, type):
        """Set the type of the attribute."""
        self.type = type
        return self

    def add_text(self, language_code, text):
        """Set text for the main element."""
        self.text.add_translation(language_code, text)
        return self
    
    def add_reserve_value(self, value):
        """Add a reserved value for the attribute."""
        self.reserved_values.append(value)
        return self

    def add_reserve_values(self, values):
        """Add some reserved values for the attribute."""
        self.reserved_values.extend(values)
        return self

    def validate(self):
        """Check if all languages have a value for the text."""
        self.text.validate()
        if not self.generate_name:
            raise ValueError("Attribute generate name is not set.")
        return self

    def to_dict(self):
        """Convert the attribute to a dictionary, including all information."""
        return {
            "id": self.id,
            "generate_name": self.generate_name,
            "type": self.type,
            "text": self.text.to_dict(),
            "reserved_values": [
                value.to_dict() for value in self.reserved_values
            ],
        }
