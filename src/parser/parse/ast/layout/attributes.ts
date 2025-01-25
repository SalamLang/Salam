import { AstNode } from '../node';
import { AstLayoutAttribute } from './attribute';

export class AstLayoutAttributes extends AstNode {
    attributes: AstLayoutAttribute[];

    constructor() {
        super("LayoutAttributes");
        this.attributes = [];
    }

    exists(attribute: AstLayoutAttribute): boolean {
        for (let attr of this.attributes) {
            if (attr.getCheckSum() === attribute.getCheckSum()) {
                return true;
            }
        }
        return false;
    }

    styleExists(attribute: AstLayoutAttribute): boolean {
        for (let attr of this.attributes) {
            if (attr.isStyle() && attr.getCheckSum() === attribute.getCheckSum()) {
                return true;
            }
        }
        return false;
    }

    normalExists(attribute: AstLayoutAttribute): boolean {
        for (let attr of this.attributes) {
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
            this.attributes.push(attribute);
            return true;
        }
    }
}
