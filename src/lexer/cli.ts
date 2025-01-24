import fs from 'fs';
import path from 'path';
import { languageMaps, languageMapsValues } from './language/data'; 
import { LanguageMap } from './language/language';
import { SALAM_VERSION } from './../base';
import { getLanguageIDByFlag } from './language/utils';

const lang_flag = "--lang";

export function processCommandRun(fileName: string | undefined, absoluteDirPath: string | undefined, code: string, selectedLanguage: LanguageMap): void {
    console.log(`Processing code in ${selectedLanguage.name}:\n${code}`);
    console.log(fileName);
    console.log(absoluteDirPath);
};

export function processLanguageFlag(args: string[], selectedLanguage: LanguageMap): number {
    const langFlagIndex: number = args.findIndex((arg: string) => arg === lang_flag);

    if (langFlagIndex !== -1) {
        const langValue: string | undefined = args[langFlagIndex + 1];

        if (!langValue) {
            console.error(`Error: Missing value for ${lang_flag} flag.`);
            showAvailableLanguages();
            return 1;
        }

        const languageId = getLanguageIDByFlag(langValue);

        if (languageId) {
            const language = languageMaps[languageId];
            Object.assign(selectedLanguage, language);
        } else {
            console.error(`Error: Invalid language value "${langValue}".`);
            showAvailableLanguages();
            return 1;
        }
    }

    return 0;
};

export function handleMissingArguments(): number {
    console.error("Error: Missing arguments. Please provide at least one argument.");
    showUsage();
    return 1;
};

export function processCommand(args: string[], selectedLanguage: LanguageMap): number {
    const command: string | undefined = args.find((arg: string, index: number) => {
        const previousArg: string | undefined = args[index - 1];
        return !arg.startsWith("--") && previousArg !== lang_flag;
    });

    if (!command) {
        console.error("Error: No command provided.");
        return handleMissingArguments();
    }
    else if (command === "version") {
        if (hasExtraArguments(args)) {
            return 1;
        }
        console.log("Salam CLI, version " + SALAM_VERSION);
        return 0;
    }
    else if (command === "help") {
        if (hasExtraArguments(args)) {
            return 1;
        }
        showUsage();
        return 0;
    }
    else if (command === "file") {
        const filePath = args[args.indexOf("file") + 1] || undefined;

        if (!filePath) {
            console.error(`Error: File path not provided.`);
            return 1;
        }
        else if (!fs.existsSync(filePath)) {
            console.error(`Error: File not found - "${filePath}"`);
            return 1;
        }

        const fileName = fs.realpathSync(filePath);
        const absoluteDirPath = fs.realpathSync(path.dirname(filePath));
    
        const code = fs.readFileSync(fileName, 'utf8');
    
        processCommandRun(fileName, absoluteDirPath, code, selectedLanguage);
    }
    else if (command === "code") {
        const code = args[args.indexOf("code") + 1];

        if (!code) {
            console.error("Error: Code not provided.");
            return 1;
        }

        processCommandRun(undefined, undefined, code, selectedLanguage);
    }
    else {
        console.error("Error: Invalid command.");
        showUsage();
        return 1;
    }

    return 0;
};

export function hasExtraArguments(args: string[]): boolean {
    const command: string | undefined = args.find((arg: string) => !arg.startsWith("--") && arg !== lang_flag);
    
    if (args.length > 2) {
        const commandName: string = command ? `"${command}"` : "This command";
        console.error(`Error: ${commandName} does not accept additional arguments.`);
        return true;
    }
    return false;
};

export function showAvailableLanguages(): void {
    console.log("Available languages:");
    languageMapsValues.forEach((lang: LanguageMap) => console.log(`  ${lang.flag} - ${lang.name}`));
};

export function showUsage(): void {
    console.log("Usage: ./salam [options] <command>");
    console.log("");
    console.log("Commands:");
    console.log("  version         Display the version of Salam CLI.");
    console.log("  help            Display this help message.");
    console.log("  file <path>     Process a file with the specified path.");
    console.log("  code <value>    Process the provided code snippet.");
    console.log("");
    console.log("Options:");
    console.log(`  ${lang_flag} <value>    Select the language to use.`);
    showAvailableLanguages();
};
