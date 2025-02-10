import { processCommandRun } from '../run/run';
import { LanguageMap } from './../../language/language';

export const code_flag: string = "code";

export function processCommandCode(type: string, args: string[], selectedLanguage: LanguageMap): number {
    const source: string = args[args.indexOf(code_flag) + 1];

    if (! source) {
        console.error("Error: Code not provided.");
        return 1;
    }

    return processCommandRun(type, undefined, undefined, source, selectedLanguage);
};
