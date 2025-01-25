import { AstNode } from './node';
import { AstType } from './type';

export class AstFunctionParameter extends AstNode {
    name: string;
    value_type: AstType;

    constructor(name: string, value_type: AstType) {
        super("FunctionParameter");
        this.name = name;
        this.value_type = value_type;
    }
};
