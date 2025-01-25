import { LanguageID, LanguageMap } from "./language";

export const languageMaps: Record<LanguageID, LanguageMap> = {
    [LanguageID.English]: {
        id: LanguageID.English,
        name: "English",
        local_name: "English",
        flag: "en",
        keywords: [
            "if",
            "else",
            "while"
        ],
    },
    [LanguageID.Persian]: {
        id: LanguageID.Persian,
        name: "Persian",
        local_name: "فارسی",
        flag: "fa",
        keywords: [
            "اگر",
            "درغیراینصورت",
            "وقتی"
        ],
    }
};

export const languageMapsValues = Object.values(languageMaps);
