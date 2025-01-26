import { AstNode } from '../node';
import { AstLayoutAttributes } from './attributes';
import { stringify } from '../../../../../serializer';

export class AstLayout extends AstNode {
    enduser_name: string;
    generate_name: string | undefined = undefined;
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
