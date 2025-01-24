export function handleMissingArguments(): number {
    console.error("Error: Missing arguments. Please provide at least one argument.");
    showUsage();
    return 1;
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