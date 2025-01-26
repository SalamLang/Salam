import { AstNode } from '../node';
import { AstLayoutElement } from './element';
import { stringify } from '../../../../../serializer';

export class AstLayout extends AstNode {
    root: AstLayoutElement;

    constructor(root: AstLayoutElement) {
        super("Layout");
        this.root = root;
    }

    print(): void {
        console.log(this.stringify());
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            root: this.root.stringify(wantsJson),
        };
        return stringify(obj, wantsJson);
    }
};
