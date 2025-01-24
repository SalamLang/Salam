"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.processCommandFile = processCommandFile;
const fs_1 = __importDefault(require("fs"));
const path_1 = __importDefault(require("path"));
const run_1 = require("./../run");
function processCommandFile(args, selectedLanguage) {
    const filePath = args[args.indexOf("file") + 1] || undefined;
    let source, fileName, absoluteDirPath;
    try {
        if (!filePath) {
            console.error(`Error: File path not provided.`);
            return 1;
        }
        else if (!fs_1.default.existsSync(filePath)) {
            console.error(`Error: File not found - "${filePath}"`);
            return 1;
        }
        fileName = fs_1.default.realpathSync(filePath);
        absoluteDirPath = fs_1.default.realpathSync(path_1.default.dirname(filePath));
        source = fs_1.default.readFileSync(fileName, 'utf8');
    }
    catch (error) {
        if (error.code === 'ENOENT') {
            console.error(`Error: File not found - "${filePath}"`);
        }
        else {
            console.error(`Error reading file: ${error.message}`);
        }
        return 1;
    }
    return (0, run_1.processCommandRun)(fileName, absoluteDirPath, source, selectedLanguage);
}
;
