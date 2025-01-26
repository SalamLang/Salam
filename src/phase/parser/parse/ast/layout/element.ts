import { AstNode } from '../node';
import { AstLayoutBlock } from './block';
import { AstLayoutAttributes } from './attributes';
import { stringify } from '../../../../../serializer';

export class AstLayoutElement extends AstNode {
    enduser_name: string;
    generate_name: string | undefined;
    generate_type: string | undefined;
    parent_generate_name: string | undefined;
    parent_generate_type: string | undefined;
	block: AstLayoutBlock;
    content: string | undefined;
    attributes: AstLayoutAttributes;
    globalAttributes: AstLayoutAttributes;

    constructor(enduser_name: string) {
        super("LayoutElement");
        this.enduser_name = enduser_name;
        this.block = new AstLayoutBlock();
        this.attributes = new AstLayoutAttributes();
        this.globalAttributes = new AstLayoutAttributes();
        this.parent_generate_name = undefined;
        this.parent_generate_type = undefined;
        this.generate_name = undefined;
        this.generate_type = undefined;
        this.content = undefined;
    }

    print(): void {
        console.log(this.stringify());
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            enduser_name: this.enduser_name,
            generate_name: this.generate_name,
            generate_type: this.generate_type,
            block: this.block.stringify(false),
            attributes: this.attributes.stringify(false),
            globalAttributes: this.globalAttributes.stringify(false),
        };
        return stringify(obj, wantsJson);
    }
}
