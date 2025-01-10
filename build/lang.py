class Lang:
    languages = {}

    @classmethod
    def add_lang(self, name, full_name, international_name):
        """Add a new language to the list of supported languages."""
        self.languages[name] = {
            "code": name,
            "full_name": full_name,
            "international_name": international_name,
        }
        setattr(self, name, name)

    @classmethod
    def get_languages(self):
        """Get the list of supported languages."""
        return self.languages

    def __getattr__(self, name):
        """Get the object for a specific language."""
        return self.languages.get(name.upper(), None)


# Add supported languages
Lang.add_lang("EN", "English", "English")
Lang.add_lang("FA", "Persian", "فارسی")
# Lang.add_lang("ES", "Spanish", "Español")
# Lang.add_lang("AR", "Arabic", "العربية")
