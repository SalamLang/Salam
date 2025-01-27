import { processCommandRun } from './../../../../common/cli/parser';
import { LanguageMap } from './../../../../common/language/language';

export const code_flag = "code";

export function processCommandCode(args: string[], selectedLanguage: LanguageMap): number {
    const source = args[args.indexOf(code_flag) + 1];

    if (! source) {
        console.error("Error: Code not provided.");
        return 1;
    }

    return processCommandRun(undefined, undefined, source, selectedLanguage);
};
