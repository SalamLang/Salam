import { processCommandCode } from './command/code';
import { processCommandFile } from './command/file';
import { lang_flag } from '../../../common/cli/language';
import { processCommandVersion } from './command/version';
import { processCommandHelp, showUsage } from './command/help';
import { LanguageMap } from '../../../common/language/language';
import { handleMissingArguments } from '../../../common/cli/error';

export function processCommand(args: string[], selectedLanguage: LanguageMap): number {
    const command: string | undefined = args.find((arg: string, index: number) => {
        const previousArg: string | undefined = args[index - 1];
        return !arg.startsWith("--") && previousArg !== lang_flag;
    });

    if (! command) {
        console.error("Error: No command provided.");
        return handleMissingArguments();
    }
    else if (command === "version") {
        return processCommandVersion(args);
    }
    else if (command === "help") {
        return processCommandHelp(args);
    }
    else if (command === "file") {
        return processCommandFile(args, selectedLanguage);
    } else if (command === "code") {
        return processCommandCode(args, selectedLanguage);
    }
    else {
        console.error("Error: Invalid command.");
        showUsage();
        return 1;
    }
};
