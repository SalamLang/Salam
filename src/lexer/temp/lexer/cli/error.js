"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.handleMissingArguments = handleMissingArguments;
exports.hasExtraArguments = hasExtraArguments;
const language_1 = require("./language");
const help_1 = require("./command/help");
function handleMissingArguments() {
    console.error("Error: Missing arguments. Please provide at least one argument.");
    (0, help_1.showUsage)();
    return 1;
}
;
function hasExtraArguments(args) {
    const command = args.find((arg) => !arg.startsWith("--") && arg !== language_1.lang_flag);
    if (args.length > 2) {
        const commandName = command ? `"${command}"` : "This command";
        console.error(`Error: ${commandName} does not accept additional arguments.`);
        return true;
    }
    return false;
}
;
