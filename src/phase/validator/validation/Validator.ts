import { RuntimeElement } from '../../../runtime/element';
import { runtimeElements } from '../../../runtime/runtime';
import { AstProgram } from "../../parser/parse/ast/program";
import { runtimeStyleAttributes } from '../../../runtime/runtime';
import { runtimeGlobalAttributes } from '../../../runtime/runtime';
import { runtimeGlobalSingleAttributes } from '../../../runtime/runtime';
import { runtimeGlobalMotherAttributes } from '../../../runtime/runtime';
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
        if (runtimeElements.length > 0) {
            for (const runtimeElementItem of runtimeElements) {
                if (runtimeElementItem.getText(this.ast.language.id)?.includes(name)) {
                    return runtimeElementItem;
                }
            }
        }
        return undefined;
    }

    getElementGlobalMotherAttributeRuntime(name: string): RuntimeElementAttribute | undefined {
        if (runtimeGlobalMotherAttributes.length > 0) {
            for (const runtimeAttributeItem of runtimeGlobalMotherAttributes) {
                if (runtimeAttributeItem.getText(this.ast.language.id)?.includes(name)) {
                    return runtimeAttributeItem;
                }
            }
        }
        return undefined;
    }

    getElementGlobalSingleAttributeRuntime(name: string): RuntimeElementAttribute | undefined {
        if (runtimeGlobalSingleAttributes.length > 0) {
            for (const runtimeAttributeItem of runtimeGlobalSingleAttributes) {
                if (runtimeAttributeItem.getText(this.ast.language.id)?.includes(name)) {
                    return runtimeAttributeItem;
                }
            }
        }
        return undefined;
    }

    getElementGlobalAttributeRuntime(name: string): RuntimeElementAttribute | undefined {
        if (runtimeGlobalAttributes.length > 0) {
            for (const runtimeAttributeItem of runtimeGlobalAttributes) {
                if (runtimeAttributeItem.getText(this.ast.language.id)?.includes(name)) {
                    return runtimeAttributeItem;
                }
            }
        }
        return undefined;
    }

    getElementStyleAttributeRuntime(name: string): RuntimeElementAttribute | undefined {
        if (runtimeStyleAttributes.length > 0) {
            for (const runtimeAttributeItem of runtimeStyleAttributes) {
                if (runtimeAttributeItem.getText(this.ast.language.id)?.includes(name)) {
                    return runtimeAttributeItem;
                }
            }
        }
        return undefined;
    }

    getElementAttributeRuntime(element: RuntimeElement, key: string): RuntimeElementAttribute | undefined {
        // TODO: handle global attributes
        for (const runtimeAttributeItem of element.attributes) {
            if (runtimeAttributeItem.getText(this.ast.language.id)?.includes(key)) {
                return runtimeAttributeItem;
            }
        }
        return undefined;
    }
};
