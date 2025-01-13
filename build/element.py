from text import Text
from prebuild import Prebuild
from attribute import Attribute
from typing import List, Optional


class Element:
    def __init__(self) -> None:
        self.text: Text = Text()
        self.attributes: List[Attribute] = []  # List of Attribute objects
        self.is_mother: bool = False
        self.generate_name: Optional[str] = None
        self.id: Optional[str] = None  # Add type for id

    def set_is_mother(self, value: bool) -> None:
        """Set the mother status for the main element."""
        self.is_mother = value

    def set_generate_name(self, generate_name: str) -> None:
        """Set the generate name for the main element."""
        self.generate_name = generate_name
        idtok = Prebuild.idize(generate_name.upper())
        self.id = "AST_LAYOUT_TYPE_" + idtok

    def add_text(self, language_code: str, text: str) -> None:
        """Set text for the main element."""
        self.text.add_translation(language_code, text)

    def add_attribute(self) -> Attribute:
        """Add an attribute with its own text translations."""
        attribute = Attribute(self)
        self.attributes.append(attribute)
        return attribute

    def to_dict(self) -> dict:
        """Convert the element to a dictionary, including all attributes."""
        for attr in self.attributes:
            attr.validate()
        return {
            "id": self.id,
            "is_mother": self.is_mother,
            "generate_name": self.generate_name,
            "text": self.text.to_dict(),
            "attributes": [attr.to_dict() for attr in self.attributes],
        }
