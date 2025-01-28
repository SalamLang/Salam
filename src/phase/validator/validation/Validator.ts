import { RuntimeElement } from './../../../runtime/element'; 
import { runtimeElements } from './../../../runtime/runtime';
import { AstProgram } from "./../../parser/parse/ast/program";
import { runtimeStyleStates } from './../../../runtime/runtime';
import { AstLayoutElement } from './../../parser/parse/ast/layout/element';
import { RuntimeElementAttribute } from './../../../runtime/element_attribute';
import { RuntimeElementStyleState } from './../../../runtime/element_style_state';
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

    private findInCollection<T extends RuntimeElement | RuntimeElementAttribute | RuntimeElementStyleState>(
        collection: T[],
        name: string,
        filterFn?: (item: T) => boolean
    ): T | undefined {
        return collection.find(item => {
            if (!item.getText(this.ast.language.id)?.includes(name)) return false;
            return filterFn ? filterFn(item) : true;
        });
    }

    private findElementRuntime(
        parent_element: AstLayoutElement | undefined,
        name: string
    ): RuntimeElement | undefined {
        return this.findInCollection(runtimeElements, name, (runtimeElementItem: RuntimeElement) => {
            if (!parent_element) return true;
            return (
                runtimeElementItem.belongs_to.length === 0 ||
                runtimeElementItem.belongs_to.some(
                    element => element.constructor.name === parent_element.constructor.name
                )
            );
        });
    }

    getElementStyleStateRuntime(parent_element: AstLayoutElement | undefined, name: string): RuntimeElementStyleState | undefined {
        return this.findInCollection<RuntimeElementStyleState>(runtimeStyleStates, name, (runtimeStyleStateItem: RuntimeElementStyleState) => {
            if (!parent_element) return true;
            const value = runtimeStyleStateItem.getText(this.ast.language.id);
            return value ? true : false;
        });
    }

    getElementRuntime(parent_element: AstLayoutElement | undefined, name: string): RuntimeElement | undefined {
        return this.findElementRuntime(parent_element, name);
    }

    hasElementRuntime(parent_element: AstLayoutElement | undefined, name: string): boolean {
        return this.findElementRuntime(parent_element, name) !== undefined;
    }

    getElementAttributeRuntime(element: RuntimeElement, key: string): RuntimeElementAttribute | undefined {
        return this.findInCollection(element.attributes, key);
    }

    hasElementAttributeRuntime(element: RuntimeElement, key: string): boolean {
        return this.getElementAttributeRuntime(element, key) !== undefined;
    }

    private executeChecks<T>(checks: (() => T | undefined)[]): T | undefined {
        return checks.reduce(
            (result, check) => result || check(),
            undefined as T | undefined
        );
    }

    getElementAllAttributeRuntime(runtimeElement: RuntimeElement, attribute_name: string): RuntimeElementAttribute | undefined {
        const checks = [
            () => this.getElementAttributeRuntime(runtimeElement, attribute_name),
            () => this.findInCollection(runtimeStyleAttributes, attribute_name),
            () =>
                runtimeElement.is_mother
                    ? this.findInCollection(runtimeGlobalMotherAttributes, attribute_name)
                    : this.findInCollection(runtimeGlobalSingleAttributes, attribute_name),
            () => this.findInCollection(runtimeGlobalAttributes, attribute_name),
        ];
        return this.executeChecks<RuntimeElementAttribute>(checks);
    }

    hasElementAllAttributeRuntime(runtimeElement: RuntimeElement, attribute_name: string): boolean {
        return this.getElementAllAttributeRuntime(runtimeElement, attribute_name) !== undefined;
    }
}
