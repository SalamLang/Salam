import { languageMaps } from '../language/data'; 
import { showAvailableLanguages } from './command/help';
import { getLanguageIDByFlag } from '../language/utils';
import { LanguageID, LanguageMap } from '../language/language';

export const lang_flag: string = "--lang";

export function processLanguageFlag(args: string[], selectedLanguage: LanguageMap): number {
    const langFlagIndex: number = args.findIndex((arg: string) => arg === lang_flag);

    if (langFlagIndex !== -1) {
        const langValue: string | undefined = args[langFlagIndex + 1];

        if (! langValue) {
            console.error(`Error: Missing value for ${lang_flag} flag.`);
            showAvailableLanguages();
            return 1;
        }

        const languageId: LanguageID | undefined = getLanguageIDByFlag(langValue);

        if (languageId !== undefined) {
            const language: LanguageMap = languageMaps[languageId];
            Object.assign(selectedLanguage, language);
            return 0;
        } else {
            console.error(`Error: Invalid language value "${langValue}".`);
            showAvailableLanguages();
            return 1;
        }
    }

    return -1;
};
