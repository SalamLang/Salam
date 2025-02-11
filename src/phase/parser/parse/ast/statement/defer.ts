import { AstNode } from '../node';
import { AstStatement } from './statement';
import { stringify } from '../../../../../serializer';

export class AstStatementDefer extends AstNode {
    statement: AstStatement;
  
    constructor(statement: AstStatement) {
        super("Defer");
        this.statement = statement;
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            statement: this.statement,
        };
        return stringify(obj, wantsJson);
    }
}
