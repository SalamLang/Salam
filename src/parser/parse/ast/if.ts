import { AstNode } from './node';

export class AstIf extends AstNode {
    condition: AstNode;
    thenBranch: AstNode;
    elseBranch?: AstNode;
  
    constructor(condition: AstNode, thenBranch: AstNode, elseBranch?: AstNode) {
        super("If");
        this.condition = condition;
        this.thenBranch = thenBranch;

        if (elseBranch) this.elseBranch = elseBranch;
    }
}
