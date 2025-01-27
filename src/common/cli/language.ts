import { languageMaps } from './../language/data'; 
import { LanguageMap } from './../language/language';
import { getLanguageIDByFlag } from './../language/utils';
import { showAvailableLanguages } from './../../phase/lexer/cli/command/help';

export const lang_flag = "--lang";

export function processLanguageFlag(args: string[], selectedLanguage: LanguageMap): number {
    const langFlagIndex: number = args.findIndex((arg: string) => arg === lang_flag);

    if (langFlagIndex !== -1) {
        const langValue: string | undefined = args[langFlagIndex + 1];

        if (! langValue) {
            console.error(`Error: Missing value for ${lang_flag} flag.`);
            showAvailableLanguages();
            return 1;
        }

        const languageId = getLanguageIDByFlag(langValue);

        if (languageId !== undefined) {
            const language = languageMaps[languageId];
            Object.assign(selectedLanguage, language);
        } else {
            console.error(`Error: Invalid language value "${langValue}".`);
            showAvailableLanguages();
            return 1;
        }
    }

    return 0;
};
