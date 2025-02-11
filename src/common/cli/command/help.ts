import { lang_flag } from './../language';
import { hasExtraArguments } from './../error';
import { LanguageMap } from './../../language/language';
import { languageMapsValues } from './../../language/data'; 

export function showAvailableLanguages(): void {
    console.log("Available languages:");
    languageMapsValues.forEach((lang: LanguageMap) => console.log(`  ${lang.flag} - ${lang.name}`));
};

export function showUsage(type: string): void {
    console.log(`Usage: ./salam-${type} [options] <command>\n
        Commands:
          version         Display the version of Salam CLI.
          help            Display this help message.
          file <path>     Process a file with the specified path.
          code <value>    Process the provided code snippet.\n
        Options:
          ${lang_flag} <value>    Select the language to use.`);
    showAvailableLanguages();
};

export function processCommandHelp(type: string, args: string[]): number {
    if (hasExtraArguments(args)) {
        return 1;
    }
    showUsage(type);
    return 0;
};
