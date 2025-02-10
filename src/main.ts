import { processCommand } from './common/cli/cli';
import { processLanguageFlag } from './common/cli/language';
import { languageMapsValues } from './common/language/data';

export const phases: string[] = [
    "lexer",
    "parser",
    "validator",
    "generator",
];

export const finalPhase: string = phases[phases.length - 1];

function displayHelp() {
    console.log("You need to choose a phase of the Salam programming language:");
    phases.forEach(phase => {
        console.log(`    ${phase}`);
    });
    console.log("");
}

function main(args: string[]): number {
    if (args.length === 0) {
        displayHelp();
        return 1;
    }

    const selectedLanguage = languageMapsValues[0];

    const languageResult: number = processLanguageFlag(args, selectedLanguage);
    if (languageResult === 1) {
        return languageResult;
    }

    const selectedPhase: string = args[0];
    if (!phases.includes(selectedPhase)) {
        return processCommand(finalPhase, args, selectedLanguage, languageResult);
    }

    const remainingArgs: string[] = args.slice(1);
    
    return processCommand(selectedPhase, remainingArgs, selectedLanguage, languageResult);
}

const args: string[] = process?.argv?.slice(2) || [];
process.exit(main(args));
