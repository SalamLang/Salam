import { SALAM_VERSION } from './../../base';
import { hasExtraArguments } from './../cli';

export function processCommandVersion(args: string[]): number {
    if (hasExtraArguments(args)) {
        return 1;
    }
    console.log("Salam CLI, version " + SALAM_VERSION);
    return 0;
};

