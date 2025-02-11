import { AstStatement } from './statement';
import { AstType } from '../expression/type';
import { AstExpression } from '../expression/expression';

export class AstStatementExpressionDeclaration extends AstStatement {
    value_type: AstType;
    name: string;
    value: AstExpression | undefined;
    
    constructor(value_type: AstType, name: string, value: AstExpression | undefined) {
        super("ExpressionDeclaration");
        this.value_type = value_type;
        this.name = name;
        this.value = value;
    }
}
