import fs from 'fs';
import path from 'path';

import { processCommandRun } from './../../../../common/cli/generator';
import { LanguageMap } from './../../../../common/language/language';

export function processCommandFile(args: string[], selectedLanguage: LanguageMap): number {
    const filePath: string | undefined = args[args.indexOf("file") + 1] || undefined;

    let source, fileName, absoluteDirPath;

    try {
        if (! filePath) {
            console.error(`Error: File path not provided.`);
            return 1;
        } else if (! fs.existsSync(filePath)) {
            console.error(`Error: File not found - "${filePath}"`);
            return 1;
        }

        fileName = fs.realpathSync(filePath);
        absoluteDirPath = fs.realpathSync(path.dirname(filePath));

        source = fs.readFileSync(fileName, 'utf8');
    } catch (error: unknown) {
        if (error instanceof Error) {
            console.error(`Error reading file: ${error.message}`);
        } else {
            console.error(`Error: File not found - "${filePath}"`);
        }
        return 1;
    }

    return processCommandRun(fileName, absoluteDirPath, source, selectedLanguage);
};
