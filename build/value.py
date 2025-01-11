from text import Text
from lang import Lang
from prebuild import Prebuild


class Value:
    def __init__(self):
        self.text = Text()
        self.generate_name = None
        self.is_function = False

    def set_generate_name(self, generate_name):
        """Set the generate name for the attribute value"""
        self.generate_name = generate_name
        idtok = Prebuild.idize(generate_name.upper())
        self.id = "AST_LAYOUT_ATTRIBUTE_VALUE_TYPE_" + idtok
        return self

    def set_is_function(self, value):
        """Set the value is a function."""
        self.is_function = value
        return self

    def add_text(self, language_code, text):
        """Set text for the main element."""
        self.text.add_translation(language_code, text)
        return self

    def set_all_text(self, text):
        """Set text for all languages."""
        if isinstance(text, list):
            for lang in text:
                self.set_all_text(lang)
        elif isinstance(text, str):
            list(
                map(lambda lang: self.add_text(lang, text), Lang.get_languages().keys())
            )
        else:
            raise ValueError("The text should be a string or a list of strings.")
        return self

    def validate(self):
        """Check if all languages have a value for the text."""
        self.text.validate()
        if not self.generate_name:
            raise ValueError("The generate name is not set.")
        return self

    def to_dict(self):
        """Convert the attribute value to a dictionary, including all information."""
        return {
            "id": self.id,
            "generate_name": self.generate_name,
            "is_function": self.is_function,
            "text": self.text.to_dict(),
        }

    def to_string(self):
        """Convert the attribute value to a string, including all information."""
        return Prebuild.to_string(self.to_dict())
