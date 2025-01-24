export function isUtf8Alpha(char: string): boolean {
    return /^[_a-zA-Z\u0600-\u06FF]+$/.test(char);
};

export function isEnglishDigit(char: string): boolean {
    return /^\d+$/.test(char);
};

export function isPersianDigit(char: string): boolean {
    return /^[\u06F0-\u06F9]+$/.test(char);
};

export function isArabicDigit(char: string): boolean {
    return /^[\u0660-\u0669]+$/.test(char);
};

export function isUtf8Number(char: string): boolean {
    return /^[\d\u0660-\u0669\u06F0-\u06F9]+$/.test(char);
};

export function utf8Decode(source: string, index: number): { char: string; newIndex: number } {
    const char = source[index] || '\0';
    return {
        char,
        newIndex: index + 1
    };
};
