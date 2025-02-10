import { SALAM_VERSION } from './../../../base';
import { hasExtraArguments } from './../error';

export function processCommandVersion(type: string, args: string[]): number {
    if (hasExtraArguments(args)) {
        return 1;
    }
    console.log("Salam CLI, version " + SALAM_VERSION);
    return 0;
};
