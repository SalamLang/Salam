import { processCommandRun } from '../run/run';
import { LanguageMap } from './../../language/language';

export function processCommandFile(type: string, args: string[], selectedLanguage: LanguageMap, languageCheck: number): number {
    let fs: any = undefined;
    let path: any = undefined;
    if (typeof window === "undefined") {
        let requireFunc: any;
        try {
          requireFunc = typeof require !== "undefined" ? require : eval("require");
        } catch (error) {
          console.error("Error: Unable to obtain the require function.");
          return 1;
        }
    
        try {
          fs = requireFunc("fs");
          path = requireFunc("path");
        } catch (error) {
          console.error("Error: Unable to load 'fs' or 'path' modules.");
          return 1;
        }
    }
    if (fs === undefined || path === undefined) {
        return 1;
    }

    const filePath: string | undefined = args[args.indexOf("file") + 1] || undefined;

    let source, fileName, absoluteDirPath;

    try {
        if (!filePath) {
            console.error(`Error: File path not provided.`);
            return 1;
        } else if (!fs.existsSync(filePath)) {
            console.error(`Error: File not found - "${filePath}"`);
            return 1;
        }
    
        fileName = fs.realpathSync(filePath);
        absoluteDirPath = fs.realpathSync(path.dirname(filePath));
    
        source = fs.readFileSync(fileName, 'utf8');
    } catch (error: unknown) {
        if (error instanceof Error) {
            if ((error as NodeJS.ErrnoException).code === 'ENOENT') {
                console.error(`Error: File not found - "${filePath}"`);
            } else {
                console.error(`Error reading file: ${error.message}`);
            }
        } else {
            console.error(`Unknown error occurred.`);
        }
        return 1;
    }

    return processCommandRun(type, fileName, absoluteDirPath, source, selectedLanguage, languageCheck);
};
