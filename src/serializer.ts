export function stringify(obj: any, wantsJson: boolean = true): string | object {
    if (wantsJson) {
        // return JSON.stringify(obj, null, 0);
        return JSON.stringify(obj, null, 2);
    }
    return obj;
};
