import { AstNode } from '../node';
import { AstLayoutAttribute } from './attribute';

export class AstLayoutAttributes extends AstNode {
    items: AstLayoutAttribute[];

    constructor() {
        super("LayoutAttributes");
        this.items = [];
    }

    get length(): number {
        return this.items.length;
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
}
