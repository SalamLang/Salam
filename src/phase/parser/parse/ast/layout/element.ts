import { AstNode } from '../node';
import { AstLayout } from './layout';
import { AstLayoutAttributes } from './attributes';
import { stringify } from '../../../../../serializer';

export class AstLayoutElement extends AstNode {
    enduser_name: string;
    generate_name: string | undefined = undefined;
    generate_type: string | undefined = undefined;
	children: AstNode[];
    attributes: AstLayoutAttributes;
    globalAttributes: AstLayoutAttributes;

    constructor(enduser_name: string) {
        super("LayoutElement");
        this.enduser_name = enduser_name;
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

    print(): void {
        console.log(this.stringify());
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            enduser_name: this.enduser_name,
            generate_name: this.generate_name,
            generate_type: this.generate_type,
            children: this.children.map(child => child.stringify(false)),
            attributes: this.attributes.stringify(false),
            globalAttributes: this.globalAttributes.stringify(false),
        };
        return stringify(obj, wantsJson);
    }
}
