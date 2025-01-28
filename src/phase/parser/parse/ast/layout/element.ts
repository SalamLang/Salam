import { AstNode } from './../node';
import { AstLayoutBlock } from './block';
import { AstLayoutAttributes } from './attributes';
import { AstLayoutElementKind } from './element_kind';
import { stringify } from './../../../../../serializer';
import { Validator } from './../../../../validator/validation/validator';
import { IdentifierGenerator } from './../../../../../common/identifier-generator';

export class AstLayoutElement extends AstNode {
    enduser_name: string;
    generate_name: string | undefined;
    generate_type: string | undefined;
    parent_generate_name: string | undefined;
    parent_generate_type: string | undefined;
	block: AstLayoutBlock;
    content: string | undefined;
    attributes: AstLayoutAttributes;
    styles: AstLayoutAttributes;
    repeat: number;
    kind: AstLayoutElementKind;
    built_in_selector: string | undefined;

    constructor(enduser_name: string) {
        super("LayoutElement");
        this.enduser_name = enduser_name;
        this.block = new AstLayoutBlock();
        this.attributes = new AstLayoutAttributes();
        this.styles = new AstLayoutAttributes();
        this.parent_generate_name = undefined;
        this.parent_generate_type = undefined;
        this.generate_name = undefined;
        this.generate_type = undefined;
        this.content = undefined;
        this.repeat = 1;
        this.kind = AstLayoutElementKind.NormalElement;
        this.built_in_selector = undefined;
    }

    generateBuiltInSelector(validator: Validator): void {
        this.built_in_selector = IdentifierGenerator.get();
    }

    print(): void {
        console.log(this.stringify());
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            kind: this.kind,
            built_in_selector: this.built_in_selector,
            enduser_name: this.enduser_name,
            generate_name: this.generate_name,
            generate_type: this.generate_type,
            block: this.block.stringify(false),
            attributes: this.attributes.stringify(false),
            styles: this.styles.stringify(false),
            repeat: this.repeat,
        };
        return stringify(obj, wantsJson);
    }
}
