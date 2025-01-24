import { processCommandRun } from './../run';
import { LanguageMap } from './../language/language';

export function processCommandCode(args: string[], selectedLanguage: LanguageMap): number {
    const code = args[args.indexOf("code") + 1];

    if (!code) {
        console.error("Error: Code not provided.");
        return 1;
    }

    return processCommandRun(undefined, undefined, code, selectedLanguage);
};
