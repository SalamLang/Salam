import { AstNode } from './../node';
import { AstBlock } from './../block';
import { AstExpression } from './../expression/expression';
import { stringify } from './../../../../../serializer';

export class AstIf extends AstNode {
    condition: AstExpression;
    thenBranch: AstBlock;
    elseBranch?: AstBlock;
  
    constructor(condition: AstExpression, thenBranch: AstBlock, elseBranch?: AstBlock) {
        super("If");
        this.condition = condition;
        this.thenBranch = thenBranch;
        if (elseBranch) {
            this.elseBranch = elseBranch;
        }
    }

    print(): void {
        console.log(this.stringify());
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
