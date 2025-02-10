export function isUrl(url: string): boolean {
    try {
        new URL(url);

        return true;
    } catch (_) {
        return false;
    }
};
