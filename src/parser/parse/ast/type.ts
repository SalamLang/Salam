import { AstNode } from './node';

export class AstType extends AstNode {
    value_type: string;

    constructor(value_type: string) {
        super("Type");
        this.value_type = value_type;
    }
}
