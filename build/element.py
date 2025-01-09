from text import Text
from attribute import Attribute

class Element:
    def __init__(self):
        self.text = Text()
        self.attributes = []
        self.is_mother = False
        self.generate_name = None

    def set_is_mother(self, value):
        """Set the mother status for the main element."""
        self.is_mother = value

    def set_generate_name(self, generate_name):
        """Set the generate name for the main element."""
        self.generate_name = generate_name
        idtok = generate_name.upper().replace(" ", "_").replace("-", "_").replace("/", "_").replace(".", "_").replace("__", "_")
        self.id = "AST_LAYOUT_TYPE_" + idtok

    def set_text(self, language_code, text):
        """Set text for the main element."""
        self.text.add_translation(language_code, text)

    def add_attribute(self):
        """Add an attribute with its own text translations."""
        attribute = Attribute(self)
        self.attributes.append(attribute)
        return attribute

    def to_dict(self):
        """Convert the element to a dictionary, including all attributes."""
        for attr in self.attributes:
            attr.validate()
        return {
            "id": self.id,
            "is_mother": self.is_mother,
            "generate_name": self.generate_name,
            "text": self.text.to_dict(),
            "attributes": [
                attr.to_dict() for attr in self.attributes
            ],
        }
