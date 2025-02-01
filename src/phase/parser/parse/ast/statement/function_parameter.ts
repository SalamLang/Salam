import { AstNode } from '../node';
import { AstExpression } from "../expression/expression";

export class AstFunctionParameter extends AstNode {
    value: AstExpression;

    constructor(value: AstExpression) {
        super("FunctionParameter");
        this.value = value;
    }
}
