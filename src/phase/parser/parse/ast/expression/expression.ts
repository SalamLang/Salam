import { AstType } from './type';
import { AstNode } from './../node';

export class AstExpression extends AstNode {
    value_type: AstType | undefined;

    constructor(type: string) {
        super(type);
    }

    getString(): string {
        return "";
    }
}
