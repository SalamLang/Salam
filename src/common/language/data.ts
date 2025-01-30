import { LanguageID, LanguageMap } from "./language";

export const languageMaps: Record<LanguageID, LanguageMap> = {
    [LanguageID.LanguageEnglish]: {
        id: LanguageID.LanguageEnglish,
        name: "English",
        local_name: "English",
        flag: "en",
    },
    [LanguageID.LanguagePersian]: {
        id: LanguageID.LanguagePersian,
        name: "Persian",
        local_name: "فارسی",
        flag: "fa",
    }
};

export const languageMapsValues: LanguageMap[] = Object.values(languageMaps);
