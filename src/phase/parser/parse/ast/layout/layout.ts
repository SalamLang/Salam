import { AstNode } from '../node';
import { AstLayoutElement } from './element';
import { AstLayoutAttributes } from './attributes';
import { stringify } from '../../../../../serializer';

export class AstLayout extends AstNode {
    enduser_name: string;
	children: AstNode[] = [];
    attributes: AstLayoutAttributes;
    globalAttributes: AstLayoutAttributes;
    generate_name: string | undefined = undefined;
    generate_type: string | undefined = undefined;

    constructor(enduser_name: string) {
        super("Layout");
        this.enduser_name = enduser_name;
        this.attributes = new AstLayoutAttributes();
        this.globalAttributes = new AstLayoutAttributes();
    }

    applyToElement(element: AstLayoutElement): void {
        element.children = this.children;
        element.attributes = this.attributes;
        element.globalAttributes = this.globalAttributes;
        element.generate_name = this.generate_name;
        element.generate_type = this.generate_type;
        element.enduser_name = this.enduser_name;
    }

    print(): void {
        console.log(this.stringify());
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            enduser_name: this.enduser_name,
            generate_name: this.generate_name,
            children: this.children.map(child => child.stringify(wantsJson)),
            attributes: this.attributes.stringify(wantsJson),
            globalAttributes: this.globalAttributes.stringify(wantsJson)
        };
        return stringify(obj, wantsJson);
    }
};
