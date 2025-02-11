import { AstStatement } from './statement';
import { AstExpression } from '../expression/expression';

export class AstStatementExpression extends AstStatement {
    value: AstExpression;
    
    constructor(value: AstExpression) {
        super("Expression");
        this.value = value;
    }
}
