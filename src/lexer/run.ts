import { LanguageMap } from "./language/language";

export function processCommandRun(fileName: string | undefined, absoluteDirPath: string | undefined, code: string, selectedLanguage: LanguageMap): number {
    console.log(`Processing code in ${selectedLanguage.name}:\n${code}`);
    console.log(fileName);
    console.log(absoluteDirPath);
    return 0;
};
