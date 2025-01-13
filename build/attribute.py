from typing import Optional, List, Any, Union
from text import Text
from prebuild import Prebuild
from type import Type  # Ensure Type is imported from the correct file


class Attribute:
    def __init__(self, parent_element: Any) -> None:
        self.text: Text = Text()
        self.type: Type = Type.String  # Ensure Type.String is a valid Type
        self.reserved_values: List[Any] = []
        self.is_required: bool = False
        self.parent_element: str = parent_element.generate_name
        self.generate_name: Optional[str] = None
        self.id: Optional[str] = None

    def set_generate_name(self, generate_name: str) -> "Attribute":
        """Set the generate name for the main element."""
        self.generate_name = generate_name
        idtok = Prebuild.idize(generate_name.upper())
        self.id = "AST_LAYOUT_ATTRIBUTE_TYPE_" + idtok
        return self

    def set_is_required(self, value: bool) -> None:
        """Set the is_required value."""
        self.is_required = value

    def set_type(self, type: Type) -> "Attribute":
        """Set the type of the attribute."""
        self.type = type
        return self

    def add_text(self, language_code: str, text: str) -> "Attribute":
        """Set text for the main element."""
        self.text.add_translation(language_code, text)
        return self

    def add_reserve_value(self, value: Any) -> "Attribute":
        """Add a reserved value for the attribute."""
        self.reserved_values.append(value)
        return self

    def add_reserve_values(self, values: List[Any]) -> "Attribute":
        """Add some reserved values for the attribute."""
        self.reserved_values.extend(values)
        return self

    def validate(self) -> "Attribute":
        """Check if all languages have a value for the text."""
        self.text.validate()
        if not self.generate_name:
            raise ValueError("Attribute generate name is not set.")
        return self

    def to_dict(self) -> dict:
        """Convert the attribute to a dictionary, including all information."""
        return {
            "id": self.id,
            "parent_element": self.parent_element,
            "generate_name": self.generate_name,
            "type": self.type.value,  # Get the string value of the Enum
            "text": self.text.to_dict(),
            "is_required": self.is_required,
            "reserved_values": [value.to_dict() for value in self.reserved_values],
        }
