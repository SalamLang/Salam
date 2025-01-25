import { AstNode } from '../node';
import { AstExpression } from '../expression';

export class AstLayoutElement extends AstNode {
	children: AstNode[] = [];
    parameters: { [key: string]: AstExpression } = {};
    styleParameters: { [key: string]: AstExpression } = {};

    constructor() {
        super("LayoutElement");
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
