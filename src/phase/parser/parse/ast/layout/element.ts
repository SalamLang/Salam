import { AstNode } from '../node';
import { AstLayout } from './layout';
import { AstLayoutAttributes } from './attributes';
import { RuntimeElement } from './../../../../../runtime/element';

export class AstLayoutElement extends AstNode {
    name: string;
	children: AstNode[];
    attributes: AstLayoutAttributes;
    globalAttributes: AstLayoutAttributes;
    runtimeElement: RuntimeElement;

    constructor(name: string, runtimeElement: RuntimeElement) {
        super("LayoutElement");
        this.name = name;
        this.children = [];
        this.attributes = new AstLayoutAttributes();
        this.globalAttributes = new AstLayoutAttributes();
        this.runtimeElement = runtimeElement;
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
