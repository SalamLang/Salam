from enum import Enum
from text import Text
from prebuild import Prebuild
from attribute import Attribute
# from type import Type  # Import Type as the Enum
from value import Value
from typing import List, Optional

class PropertyType(Enum):
    """Type of an attribute."""

    Any = "AST_TYPE_ANY"
    AnyNoEmpty = "AST_TYPE_ANY"
    AnyTodo = "AST_TYPE_ANY"
    String = "AST_TYPE_STRING"
    Url = "AST_TYPE_URL"

    Sizes = "AST_TYPE_SIZES"
    Size = "AST_TYPE_SIZE"
    PositiveInt = "AST_TYPE_NUMBER_POSITIVE"
    IDs = "AST_TYPE_ANY"
    ID = "AST_TYPE_ANY"
    CSSMedia = "AST_TYPE_ANY"
    Coords = "AST_TYPE_STRING"
    Color = "AST_TYPE_STRING"
    Colors = "AST_TYPE_STRING"
    Urls = "AST_TYPE_URLS"
    Number = "AST_TYPE_NUMBER"
    # Float = "AST_TYPE_FLOAT"
    Boolean = "AST_TYPE_BOOLEAN"
    Strings = "AST_TYPE_STRING"

    # YYYY-MM-DDThh:mm:ssTZD
    Date = "AST_TYPE_DATE"
    Time = "AST_TYPE_TIME"
    Times = "AST_TYPE_TIMES"
    DateTime = "AST_TYPE_DATETIME"

    NUMBER_OR_2NUMBERS = "AST_TYPE_NUMBER_OR_2NUMBERS" # ASPECT-RATIO

class Property:
    def __init__(self) -> None:
        self.text: Text = Text()
        self.generate_name: Optional[str] = None
        self.type: PropertyType = PropertyType.String  # Use PropertyType instead of Type
        self.reserved_values: List[Value] = []

    def set_generate_name(self, generate_name: str) -> 'Property':
        """Set the generate name for the main element."""
        self.generate_name = generate_name
        idtok = Prebuild.idize(generate_name.upper())
        self.id: str = "AST_LAYOUT_TYPE_" + idtok
        return self

    def add_text(self, language_code: str, text: str) -> 'Property':
        """Set text for the main element."""
        self.text.add_translation(language_code, text)
        return self

    def set_type(self, type: PropertyType) -> 'Property':  # Change argument to PropertyType
        """Set the type of the property."""
        self.type = type
        return self

    def add_reserve_value(self, value: Value) -> 'Property':
        """Add a reserved value for the property."""
        if not isinstance(value, Value):
            raise TypeError("Reserved value must be an instance of Value.")
        self.reserved_values.append(value)
        return self

    def add_reserve_values(self, values: List[Value]) -> 'Property':
        """Add multiple reserved values for the property."""
        for value in values:
            self.add_reserve_value(value)
        return self

    def to_dict(self) -> dict:
        """Convert the element to a dictionary, including all attributes and reserved values."""
        for value in self.reserved_values:
            value.validate()
        return {
            "id": self.id,
            "generate_name": self.generate_name,
            "type": self.type.name if hasattr(self.type, 'name') else str(self.type),  # Safely access name or convert to string
            "text": self.text.to_dict(),
            "reserved_values": [value.to_dict() for value in self.reserved_values],
        }
    


property_instance = Property()
property_instance.set_generate_name("example_property")
property_instance.add_text("en", "Example Text")
property_instance.set_type(PropertyType.String)
property_dict = property_instance.to_dict()
print(property_dict)
