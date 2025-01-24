import { languageMaps } from './language/data';
import { processCommand } from './cli';
import { processLanguageFlag } from './language';

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
