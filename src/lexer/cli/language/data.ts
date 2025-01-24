import { LanguageID, LanguageMap } from "./language";

export const languageMaps: Record<LanguageID, LanguageMap> = {
    [LanguageID.LANGUAGE_ENGLISH]: {
        id: LanguageID.LANGUAGE_ENGLISH,
        name: "English",
        local_name: "English",
        flag: "en",
        keywords: [
            "if",
            "else",
            "while"
        ]
    },
    [LanguageID.LANGUAGE_PERSIAN]: {
        id: LanguageID.LANGUAGE_PERSIAN,
        name: "Persian",
        local_name: "فارسی",
        flag: "fa",
        keywords: [
            "اگر",
            "درغیراینصورت",
            "وقتی"
        ]
    }
};

export const languageMapsValues = Object.values(languageMaps);
