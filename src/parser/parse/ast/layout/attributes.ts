import { AstNode } from '../node';
import { AstExpression } from '../expression';

export class AstLayoutAttributes extends AstNode {
    parameters: { [key: string]: AstExpression } = {};
    styleParameters: { [key: string]: AstExpression } = {};

    constructor() {
        super("LayoutAttributes");
    }

    hasParameter(key: string): boolean {
        return key in this.parameters;
    }

    hasStyleParameter(key: string): boolean {
        return key in this.styleParameters;
    }

    setParameter(key: string, value: AstExpression) {
        this.parameters[key] = value;
    }

    setStyleParameter(key: string, value: AstExpression) {
        this.styleParameters[key] = value;
    }
}
