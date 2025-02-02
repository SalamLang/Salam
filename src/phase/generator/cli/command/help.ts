import { lang_flag } from './../../../../common/cli/language';
import { hasExtraArguments } from './../../../../common/cli/error';
import { LanguageMap } from './../../../../common/language/language';
import { languageMapsValues } from './../../../../common/language/data'; 

export function showAvailableLanguages(): void {
    console.log("Available languages:");
    languageMapsValues.forEach((lang: LanguageMap) => console.log(`  ${lang.flag} - ${lang.name}`));
};

export function showUsage(): void {
    console.log(`Usage: ./salam-generator [options] <command>\n
        Commands:
          version         Display the version of Salam CLI.
          help            Display this help message.
          file <path>     Process a file with the specified path.
          code <value>    Process the provided code snippet.\n
        Options:
          ${lang_flag} <value>    Select the language to use.`);
            showAvailableLanguages();
};

export function processCommandHelp(args: string[]): number {
    if (hasExtraArguments(args)) {
        return 1;
    }
    showUsage();
    return 0;
};
