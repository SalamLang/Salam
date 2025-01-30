import { AstStatement } from '../statement/statement';
import { AstExpression } from '../expression/expression';

export class AstStatementPrint extends AstStatement {
    value: AstExpression;

    constructor(value: AstExpression) {
        super("Print");
        this.value = value;
    }
}
