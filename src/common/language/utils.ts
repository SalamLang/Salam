import { languageMapsValues } from './data';
import { LanguageID, LanguageMap } from './language';

export function getLanguageIDByFlag(flag: string): LanguageID | undefined {
    return languageMapsValues.find((lang: LanguageMap) => lang.flag === flag)?.id;
};
