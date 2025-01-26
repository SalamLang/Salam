export enum MessageKeys {
    LEXER_INVALID_UNEXPECTED_CHAR,
    LEXER_UNTERMINATED_MULTI_LINE_COMMENT_BLOCK,
};

export type MessageStruct = {
    [K in MessageKeys]: string;
};

export function messageRenderer(message: string, ...args: string[]): string {
    return message.replace(/{(\d+)}/g, (match, number) => {
        return typeof args[number] !== 'undefined' ? args[number] : match;
    });
};
