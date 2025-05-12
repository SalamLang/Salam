import { AstNode } from './../node';
import { AstBlock } from './../block';
import { AstExpression } from './../expression/expression';
import { stringify } from './../../../../../serializer';

export class AstStatementIf extends AstNode {
    condition: AstExpression;
    thenBranch: AstBlock;
    elseBranch: AstBlock | undefined;
  
    constructor(condition: AstExpression, thenBranch: AstBlock, elseBranch: AstBlock | undefined) {
        super("If");
        this.condition = condition;
        this.thenBranch = thenBranch;
        if (elseBranch !== undefined) {
            this.elseBranch = elseBranch;
        } else {
            this.elseBranch = undefined;
        }
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            condition: this.condition,
            thenBranch: this.thenBranch,
            elseBranch: this.elseBranch,
        };
        return stringify(obj, wantsJson);
    }
}
