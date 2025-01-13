from typing import Dict, Any

class Lang:
    languages: Dict[str, Dict[str, str]] = {}

    @classmethod
    def add_lang(cls, name: str, full_name: str, international_name: str) -> None:
        """Add a new language to the list of supported languages."""
        cls.languages[name] = {
            "code": name,
            "full_name": full_name,
            "international_name": international_name,
        }
        setattr(cls, name, name)

    @classmethod
    def get_languages(cls) -> Dict[str, Dict[str, str]]:
        """Get the list of supported languages."""
        return cls.languages

    def __getattr__(self, name: str) -> Any:
        """Get the object for a specific language."""
        return self.languages.get(name.upper(), None)


# Add supported languages
Lang.add_lang("EN", "English", "English")
Lang.add_lang("FA", "Persian", "فارسی")
# Lang.add_lang("ES", "Spanish", "Español")
# Lang.add_lang("AR", "Arabic", "العربية")
