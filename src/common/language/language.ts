export enum LanguageID {
    LANGUAGE_ENGLISH = 0,
    LANGUAGE_PERSIAN = 1,
};

export interface LanguageMap {
    id: LanguageID;
    name: string;
    local_name: string;
    flag: string;
    keywords: string[];
}

