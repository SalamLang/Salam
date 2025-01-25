export enum LanguageID {
    English = "English",
    Persian = "Persian",
};

export interface LanguageMap {
    id: LanguageID;
    name: string;
    local_name: string;
    flag: string;
    keywords: string[];
}

