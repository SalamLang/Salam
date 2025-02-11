import { AstType } from './type';
import { AstNode } from './../node';

export type ExpressionPair = {
    key: string;
    value: string;
};
  
export class AstExpression extends AstNode {
    value_type: AstType | undefined;
    generated_value: string | undefined;

    constructor(type: string) {
        super(type);
    }

    getString(): string {
        if (this.generated_value === undefined) {
            return "";
        }
        return this.generated_value;
    }
};
