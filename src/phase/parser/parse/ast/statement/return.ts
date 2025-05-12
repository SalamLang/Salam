import { AstStatement } from '../statement/statement';
import { AstExpression } from '../expression/expression';

export class AstStatementReturn extends AstStatement {
    value: AstExpression;

    constructor(value: AstExpression) {
        super("Return");
        this.value = value;
    }
}
