import { processCommand } from './../../common/cli/cli';
import { processLanguageFlag } from '../../common/cli/language';
import { languageMapsValues } from './../../common/language/data';

function main(args: string[]): number {
    let selectedLanguage = languageMapsValues[0];

    const result: number = processLanguageFlag(args, selectedLanguage);
    if (result === 1) {
        return result;
    }

    return processCommand("generator", args, selectedLanguage, result);
};

if (typeof window === "undefined") {
    const args: string[] = process?.argv?.slice(2) || [];
    process.exit(main(args));
}
