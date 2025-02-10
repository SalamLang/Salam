import { AstNode } from '../node';
import { AstType } from '../expression/type';

export class AstFunctionArgument extends AstNode {
    name: string;
    value_type: AstType;
    is_optional: boolean;

    constructor(name: string, value_type: AstType, is_optional: boolean = false) {
        super("FunctionArgument");
        this.name = name;
        this.value_type = value_type;
        this.is_optional = is_optional;
    }
}
