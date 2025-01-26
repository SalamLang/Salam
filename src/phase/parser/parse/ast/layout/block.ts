import { AstNode } from '../node';
import { AstLayoutElement } from './element';
import { stringify } from '../../../../../serializer';

export class AstLayoutBlock extends AstNode {
    items: AstLayoutElement[];

    constructor() {
        super("LayoutBlock");
        this.items = [];
    }

    get length(): number {
        return this.items.length;
    }

    push(element: AstLayoutElement): boolean {
        this.items.push(element);
        return true;
    }

    print(): void {
        console.log(this.stringify());
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            items: this.items.map(item => item.stringify(wantsJson)),
        };
        return stringify(obj, wantsJson);
    }
}
