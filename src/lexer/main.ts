import { languageMaps } from './cli/language/data';
import { processCommand } from './cli/cli';
import { processLanguageFlag } from './cli/language';

function main(args: string[]): number {
    let selectedLanguage = languageMaps[0];

    const result = processLanguageFlag(args, selectedLanguage);
    if (result !== 0) {
        return result;
    }

    return processCommand(args, selectedLanguage);
}

const args = process?.argv?.slice(2) || [];
process.exit(main(args));
