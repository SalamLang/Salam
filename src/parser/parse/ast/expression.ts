import { AstNode } from './node';

export class AstExpression extends AstNode {
    expression: string;
  
    constructor(expression: string) {
        super("Expression");
        this.expression = expression;
    }
}
  