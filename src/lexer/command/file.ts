import fs from 'fs';
import path from 'path';

import { LanguageMap } from './../language/language';
import { processCommandRun } from './../run';

export function processCommandFile(args: string[], selectedLanguage: LanguageMap): number {
    const filePath = args[args.indexOf("file") + 1] || undefined;

    let code, fileName, absoluteDirPath;

    try {
        if (!filePath) {
            console.error(`Error: File path not provided.`);
            return 1;
        }
        else if (!fs.existsSync(filePath)) {
            console.error(`Error: File not found - "${filePath}"`);
            return 1;
        }

        fileName = fs.realpathSync(filePath);
        absoluteDirPath = fs.realpathSync(path.dirname(filePath));

        code = fs.readFileSync(fileName, 'utf8');
    } catch (error: any) {
        if (error.code === 'ENOENT') {
            console.error(`Error: File not found - "${filePath}"`);
        } else {
            console.error(`Error reading file: ${error.message}`);
        }
        return 1;
    }

    return processCommandRun(fileName, absoluteDirPath, code, selectedLanguage);
};
