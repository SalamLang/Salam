import { LanguageID, LanguageMap } from './language';
import { languageMapsValues } from './data';

export function getLanguageIDByFlag(flag: string): LanguageID | undefined {
    return languageMapsValues.find((lang: LanguageMap) => lang.flag === flag)?.id;
};

export function getLanguageNameById(id: LanguageID): string | undefined {
    return languageMapsValues.find((lang: LanguageMap) => lang.id === id)?.name;
};

