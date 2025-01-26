import { RuntimeElement } from '../../../runtime/element';
import { runtimeElements } from '../../../runtime/runtime';
import { AstProgram } from "../../parser/parse/ast/program";
import { runtimeStyleAttributes } from '../../../runtime/runtime';
import { RuntimeElementAttribute } from '../../../runtime/element_attribute';

export class Validator {
    ast: AstProgram;
    errors: string[];
    
    constructor(ast: AstProgram) {
        this.ast = ast;
        this.errors = [];
    }

    pushError(message: string) {
        this.errors.push("Validator error: " + message);
    }

    getElementRuntime(name: string): RuntimeElement | undefined {
        for (const runtimeElementItem of runtimeElements) {
            if (runtimeElementItem.getText(this.ast.language.id)?.includes(name)) {
                return runtimeElementItem;
            }
        }
        return undefined;
    }

    getElementStyleAttributeRuntime(name: string): RuntimeElementAttribute | undefined {
        for (const runtimeElementAttributeItem of runtimeStyleAttributes) {
            if (runtimeElementAttributeItem.getText(this.ast.language.id)?.includes(name)) {
                return runtimeElementAttributeItem;
            }
        }
        return undefined;
    }

    getElementAttributeRuntime(element: RuntimeElement, key: string): RuntimeElementAttribute | undefined {
        // TODO: handle global attributes
        for (const attribute of element.attributes) {
            if (attribute.getText(this.ast.language.id)?.includes(key)) {
                return attribute;
            }
        }
        return undefined;
    }
};
