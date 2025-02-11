export function stringify(obj: object | object[], wantsJson: boolean = true): string | object {
    if (wantsJson) {
        // return JSON.stringify(obj, null, 0);
        return JSON.stringify(obj, null, 2);
    }
    return obj;
};
