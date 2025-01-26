import { AstNode } from '../node';
import { AstLayoutAttribute } from './attribute';
import { stringify } from '../../../../../serializer';

export class AstLayoutAttributes extends AstNode {
    items: AstLayoutAttribute[];

    constructor() {
        super("LayoutAttributes");
        this.items = [];
    }

    get length(): number {
        return this.items.length;
    }

    getByType(type: string, default_value: any = undefined): AstLayoutAttribute | undefined |  any {
        for (let attr of this.items) {
            if (attr.generate_type === type) {
                return attr;
            }
        }
        return default_value;
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

    print(): void {
        console.log(this.stringify());
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            items: this.items.map((item: AstLayoutAttribute) => item.stringify(wantsJson)),
        };
        return stringify(obj, wantsJson);
    }
}
