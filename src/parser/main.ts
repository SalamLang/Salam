import { processCommand } from './cli/cli';
import { processLanguageFlag } from './../common/cli/language';
import { languageMapsValues } from './../common/language/data';

function main(args: string[]): number {
    let selectedLanguage = languageMapsValues[0];

    const result = processLanguageFlag(args, selectedLanguage);
    if (result !== 0) {
        return result;
    }

    return processCommand(args, selectedLanguage);
};

const args = process?.argv?.slice(2) || [];
process.exit(main(args));
