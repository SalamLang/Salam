import { AstNode } from '../node';
import { AstLayoutAttributes } from './attributes';
export class AstLayout extends AstNode {
	children: AstNode[] = [];
    attributes: AstLayoutAttributes;
    globalAttributes: AstLayoutAttributes;

    constructor() {
        super("Layout");
        this.attributes = new AstLayoutAttributes();
        this.globalAttributes = new AstLayoutAttributes();
    }

    pushElement(element: AstNode) {
        this.children.push(element);
    }
}
