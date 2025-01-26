export interface MessageStruct {
    [key: string]: string;
};

export function messageRenderer(message: string, ...args: string[]): string {
    return message.replace(/{(\d+)}/g, (match, number) => {
        return typeof args[number] !== 'undefined' ? args[number] : match;
    });
};
