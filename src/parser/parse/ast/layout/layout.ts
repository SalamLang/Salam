import { AstNode } from './../node';
import { AstExpression } from '../expression';

export class AstLayout extends AstNode {
	children: AstNode[] = [];
    globalParameters: { [key: string]: AstExpression } = {};
    parameters: { [key: string]: AstExpression } = {};
    styleParameters: { [key: string]: AstExpression } = {};

    constructor() {
        super("Layout");
    }

    setGlobalParameter(key: string, value: AstExpression) {
        this.globalParameters[key] = value;
    }

    setParameter(key: string, value: AstExpression) {
        this.parameters[key] = value;
    }

    setStyleParameter(key: string, value: AstExpression) {
        this.styleParameters[key] = value;
    }

    pushElement(element: AstNode) {
        this.children.push(element);
    }
}
