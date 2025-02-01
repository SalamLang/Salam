import { AstNode } from './../node';
import { AstExpression } from '../expression/expression';
import { AstFunctionParameter } from '../function_parameter';

export class AstFunctionCall extends AstNode {
    name: AstExpression;
    parameters: AstFunctionAttribute[];

    constructor(name: AstExpression, parameters: AstFunctionParameter[]) {
        super("FunctionCall");
        this.name = name;
        this.parameters = parameters;
    }
}
