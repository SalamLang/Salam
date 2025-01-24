export function isUtf8Alpha(char: string): boolean {
    return /^[a-zA-Z\u0600-\u06FF]+$/.test(char);
};

export function utf8Decode(source: string, index: number): { char: string; newIndex: number } {
    const char = source[index] || '\0';
    return { char, newIndex: index + 1 };
};
