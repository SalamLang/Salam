"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.processCommand = processCommand;
const code_1 = require("./command/code");
const file_1 = require("./command/file");
const version_1 = require("./command/version");
const help_1 = require("./command/help");
const error_1 = require("./error");
const language_1 = require("./language");
function processCommand(args, selectedLanguage) {
    const command = args.find((arg, index) => {
        const previousArg = args[index - 1];
        return !arg.startsWith("--") && previousArg !== language_1.lang_flag;
    });
    if (!command) {
        console.error("Error: No command provided.");
        return (0, error_1.handleMissingArguments)();
    }
    else if (command === "version") {
        return (0, version_1.processCommandVersion)(args);
    }
    else if (command === "help") {
        return (0, help_1.processCommandHelp)(args);
    }
    else if (command === "file") {
        return (0, file_1.processCommandFile)(args, selectedLanguage);
    }
    else if (command === "code") {
        return (0, code_1.processCommandCode)(args, selectedLanguage);
    }
    else {
        console.error("Error: Invalid command.");
        (0, help_1.showUsage)();
        return 1;
    }
}
;
