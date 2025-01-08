from lang import Lang

class Text:
    def __init__(self):
        self.translations = {}

    def add_translation(self, language_code, text):
        """Add or update a translation for a specific language."""
        if language_code not in self.translations:
            self.translations[language_code] = []
        self.translations[language_code].append(text)

    def validate(self):
        """Check if all languages have a value for the text."""
        langs = Lang().get_languages()

        for lang_code in langs:
            if lang_code not in self.translations:
                raise ValueError(f"Missing translation for {lang_code}")

    def to_dict(self):
        """Convert translations to dictionary format."""
        self.validate()

        return self.translations
