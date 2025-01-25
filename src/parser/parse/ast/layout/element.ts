import { AstNode } from './../node';
import { AstLayoutAttributes } from './attributes';
import { AstLayout } from './../../ast/layout/layout';

export class AstLayoutElement extends AstNode {
    name: string;
	children: AstNode[];
    attributes: AstLayoutAttributes;
    globalAttributes: AstLayoutAttributes;

    constructor(name: string) {
        super("LayoutElement");
        this.name = name;
        this.children = [];
        this.attributes = new AstLayoutAttributes();
        this.globalAttributes = new AstLayoutAttributes();
    }

    pushElement(element: AstNode) {
        this.children.push(element);
    }

    applyToLayout(ast: AstLayout): void {
        ast.attributes = this.attributes;
        ast.globalAttributes = this.globalAttributes;
        ast.children = this.children;
    }
}
