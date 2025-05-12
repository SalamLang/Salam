import { RuntimeElement } from './element';

export class RuntimeStyleElement extends RuntimeElement {
    constructor(is_mother: boolean, generate_name: string | undefined) {
        super(is_mother, generate_name);
    }
};
