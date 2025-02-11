import { AstNode } from './../node';
import { AstLayoutAttribute } from './attribute';
import { stringify } from './../../../../../serializer';

export class AstLayoutAttributes extends AstNode {
    items: AstLayoutAttribute[];

    constructor() {
        super("LayoutAttributes");
        this.items = [];
    }

    get length(): number {
        return this.items.length;
    }

    getByGenerateName(generate_name: string): AstLayoutAttribute | undefined {
        for (let attr of this.items) {
            if (attr.generate_name === generate_name) {
                return attr;
            }
        }
        return undefined;
    }

    exists(attribute: AstLayoutAttribute): boolean {
        for (let attr of this.items) {
            if (attr.getCheckSum() === attribute.getCheckSum()) {
                return true;
            }
        }
        return false;
    }

    styleExists(attribute: AstLayoutAttribute): boolean {
        for (let attr of this.items) {
            if (attr.isStyle() && attr.getCheckSum() === attribute.getCheckSum()) {
                return true;
            }
        }
        return false;
    }

    normalExists(attribute: AstLayoutAttribute): boolean {
        for (let attr of this.items) {
            if (attr.isNormal() && attr.getCheckSum() === attribute.getCheckSum()) {
                return true;
            }
        }
        return false;
    }

    push(attribute: AstLayoutAttribute): boolean {
        if (this.exists(attribute)) {
            return false;
        } else {
            this.items.push(attribute);
            return true;
        }
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            items: this.items.map((item: AstLayoutAttribute) => item.stringify(false)),
        };
        return stringify(obj, wantsJson);
    }
}
