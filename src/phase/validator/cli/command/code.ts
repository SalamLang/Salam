import { LanguageMap } from './../../../../common/language/language';
import { processCommandRun } from './../../../../common/cli/validator';

export const code_flag: string = "code";

export function processCommandCode(args: string[], selectedLanguage: LanguageMap): number {
    const source: string = args[args.indexOf(code_flag) + 1];

    if (! source) {
        console.error("Error: Code not provided.");
        return 1;
    }

    return processCommandRun(undefined, undefined, source, selectedLanguage);
};
