import { RuntimeElement } from './../../../runtime/element'; 
import { runtimeElements } from './../../../runtime/runtime';
import { AstProgram } from "./../../parser/parse/ast/program";
import { LanguageID } from '../../../common/language/language';
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

    private static findInCollection<T extends RuntimeElement | RuntimeElementAttribute | RuntimeElementStyleState>(
        languageId: LanguageID,
        collection: T[],
        name: string,
        filterFn?: (item: T) => boolean
    ): T | undefined {
        return collection.find(item => {
            if (!item.getText(languageId)?.includes(name)) return false;
            return filterFn ? filterFn(item) : true;
        });
    }

    private static findElementRuntime(
        languageId: LanguageID,
        parent_element: AstLayoutElement | undefined,
        name: string
    ): RuntimeElement | undefined {
        return Validator.findInCollection(languageId, runtimeElements, name, (runtimeElementItem: RuntimeElement) => {
            if (!parent_element) return true;
            return (
                runtimeElementItem.belongs_to.length === 0 ||
                runtimeElementItem.belongs_to.some(
                    element => element.constructor.name === parent_element.constructor.name
                )
            );
        });
    }

    static getElementStyleStateRuntime(languageId: LanguageID, parent_element: AstLayoutElement | undefined, name: string): RuntimeElementStyleState | undefined {
        return Validator.findInCollection<RuntimeElementStyleState>(languageId, runtimeStyleStates, name, (runtimeStyleStateItem: RuntimeElementStyleState) => {
            if (!parent_element) return true;
            const value = runtimeStyleStateItem.getText(languageId);
            return value ? true : false;
        });
    }

    static getElementRuntime(languageId: LanguageID, parent_element: AstLayoutElement | undefined, name: string): RuntimeElement | undefined {
        return Validator.findElementRuntime(languageId, parent_element, name);
    }

    static hasElementRuntime(languageId: LanguageID, parent_element: AstLayoutElement | undefined, name: string): boolean {
        return Validator.findElementRuntime(languageId, parent_element, name) !== undefined;
    }

    static getElementAttributeRuntime(languageId: LanguageID, element: RuntimeElement, key: string): RuntimeElementAttribute | undefined {
        return Validator.findInCollection(languageId, element.attributes, key);
    }

    static hasElementAttributeRuntime(languageId: LanguageID, element: RuntimeElement, key: string): boolean {
        return Validator.getElementAttributeRuntime(languageId, element, key) !== undefined;
    }

    private static executeChecks<T>(checks: (() => T | undefined)[]): T | undefined {
        return checks.reduce(
            (result, check) => result || check(),
            undefined as T | undefined
        );
    }

    static getElementAttributeStyle(languageId: LanguageID, parent_element: AstLayoutElement | undefined, name: string): RuntimeElementAttribute | undefined {
        return Validator.findInCollection(languageId, runtimeStyleAttributes, name);
    }

    static getElementAllAttributeRuntime(languageId: LanguageID, runtimeElement: RuntimeElement, attribute_name: string): RuntimeElementAttribute | undefined {
        const checks = [
            () => Validator.getElementAttributeRuntime(languageId, runtimeElement, attribute_name),
            () => Validator.findInCollection(languageId, runtimeStyleAttributes, attribute_name),
            () => runtimeElement.is_mother
                        ? Validator.findInCollection(languageId, runtimeGlobalMotherAttributes, attribute_name)
                        : Validator.findInCollection(languageId, runtimeGlobalSingleAttributes, attribute_name),
            () => Validator.findInCollection(languageId, runtimeGlobalAttributes, attribute_name),
        ];
        return Validator.executeChecks<RuntimeElementAttribute>(checks);
    }

    static hasElementAllAttributeRuntime(languageId: LanguageID, runtimeElement: RuntimeElement, attribute_name: string): boolean {
        return Validator.getElementAllAttributeRuntime(languageId, runtimeElement, attribute_name) !== undefined;
    }

    getLanguageId(): LanguageID {
        return this.ast.language.id;
    }
}
