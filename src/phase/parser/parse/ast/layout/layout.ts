import { AstNode } from '../node';
import { AstLayoutAttributes } from './attributes';

export class AstLayout extends AstNode {
    enduser_name: string;
	children: AstNode[] = [];
    attributes: AstLayoutAttributes;
    globalAttributes: AstLayoutAttributes;

    constructor(enduser_name: string) {
        super("Layout");
        this.enduser_name = enduser_name;
        this.attributes = new AstLayoutAttributes();
        this.globalAttributes = new AstLayoutAttributes();
    }

    pushElement(element: AstNode) {
        this.children.push(element);
    }
};
