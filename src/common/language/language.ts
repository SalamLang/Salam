export enum LanguageID {
    LanguageEnglish = "English",
    LanguagePersian = "Persian",
};

export interface LanguageMap {
    id: LanguageID;
    name: string;
    local_name: string;
    flag: string;
};
