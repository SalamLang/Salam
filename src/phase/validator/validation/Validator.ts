import { RuntimeElement } from './../../../runtime/element';
import { runtimeElements } from './../../../runtime/runtime';
import { AstProgram } from "./../../parser/parse/ast/program";
import { AstLayoutElement } from './../../parser/parse/ast/layout/element';
import { RuntimeElementAttribute } from './../../../runtime/element_attribute';
import { runtimeStyleAttributes, runtimeGlobalAttributes, runtimeGlobalSingleAttributes, runtimeGlobalMotherAttributes } from './../../../runtime/runtime';

export class Validator {
    ast: AstProgram;
    errors: string[];

    constructor(ast: AstProgram) {
        this.ast = ast;
        this.errors = [];
    }

    pushError(message: string) {
        this.errors.push(message);
    }

    private findInCollection<T extends RuntimeElement | RuntimeElementAttribute>(
        collection: T[],
        name: string,
        filterFn?: (item: T) => boolean
    ): T | undefined {
        return collection.find((item: T) => {
            if (!item.getText(this.ast.language.id)?.includes(name)) return false;
            return filterFn ? filterFn(item) : true;
        });
    }

    getElementRuntime(parent_element: AstLayoutElement | undefined, name: string): RuntimeElement | undefined {
        return this.findInCollection(runtimeElements, name, runtimeElementItem => {
            if (!parent_element) return true;
            return (
                runtimeElementItem.belongs_to.length === 0 ||
                runtimeElementItem.belongs_to.some(
                    element => element.constructor.name === parent_element.constructor.name
                )
            );
        });
    }

    getElementGlobalMotherAttributeRuntime(name: string): RuntimeElementAttribute | undefined {
        return this.findInCollection(runtimeGlobalMotherAttributes, name);
    }

    getElementGlobalSingleAttributeRuntime(name: string): RuntimeElementAttribute | undefined {
        return this.findInCollection(runtimeGlobalSingleAttributes, name);
    }

    getElementGlobalAttributeRuntime(name: string): RuntimeElementAttribute | undefined {
        return this.findInCollection(runtimeGlobalAttributes, name);
    }

    getElementStyleAttributeRuntime(name: string): RuntimeElementAttribute | undefined {
        return this.findInCollection(runtimeStyleAttributes, name);
    }

    getElementAttributeRuntime(element: RuntimeElement, key: string): RuntimeElementAttribute | undefined {
        return this.findInCollection(element.attributes, key);
    }
}
