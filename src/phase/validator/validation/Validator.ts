import { SymbolTable } from './symbol-table';
import { RuntimeElement } from '../../../runtime/element'; 
import { runtimeElements } from '../../../runtime/runtime';
import { AstProgram } from "../../parser/parse/ast/program";
import { LanguageID } from '../../../common/language/language';
import { runtimeStyleStates } from '../../../runtime/runtime';
import { AstType } from '../../parser/parse/ast/expression/type';
import { runtimeStyleElements } from '../../../runtime/runtime';
import { AstLayoutElement } from '../../parser/parse/ast/layout/element';
import { validatorMessageRenderer } from '../../../common/message/message';
import { RuntimeElementAttribute } from '../../../runtime/element_attribute';
import { RuntimeElementStyleState } from '../../../runtime/element_style_state';
import { ValidatorMessageKeys } from '../../../common/message/validator/validator';
import { runtimeStyleAttributes, runtimeGlobalAttributes, runtimeGlobalSingleAttributes, runtimeGlobalMotherAttributes } from '../../../runtime/runtime';

export class Validator {
    ast: AstProgram;
    errors: string[];
    symbol_table: SymbolTable;
    extendedFunctions: Record<string, AstType>;
    extendedVariables: Record<string, AstType>;
    packages: Record<string, AstType>;
    packageFunctions: AstType[];

    constructor(ast: AstProgram) {
        this.ast = ast;
        this.errors = [];
        this.symbol_table = new SymbolTable();
        this.extendedFunctions = {};
        this.extendedVariables = {};
        this.packages = {};
        this.packageFunctions = [];
    }

    pushPackage(name: string, type: AstType): void {
        this.packages[name] = type;
    }

    pushExtendedFunction(name: string, type: AstType): void {
        this.extendedFunctions[name] = type;
    }

    pushExtendedVariable(name: string, type: AstType): void {
        this.extendedVariables[name] = type;
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
        let parent_runtime_element: RuntimeElement | undefined = undefined;
        if (parent_element !== undefined) {
            parent_runtime_element = Validator.getElementRuntime(languageId, undefined, parent_element.enduser_name);
        }
        return Validator.findInCollection(languageId, runtimeElements, name, (runtimeElementItem: RuntimeElement) => {
            if (!parent_element) {
                return true;
            }
            return (
                runtimeElementItem.belongs_to.length === 0 ||
                parent_runtime_element === undefined ||
                runtimeElementItem.belongs_to.some(
                    element => element.constructor.name === parent_element.constructor.name
                )
            );
        });
    }

    private static findStyleElementRuntime(
        languageId: LanguageID,
        parent_element: AstLayoutElement | undefined,
        name: string
    ): RuntimeElement | undefined {
        let parent_runtime_element: RuntimeElement | undefined = undefined;
        if (parent_element !== undefined) {
            parent_runtime_element = Validator.getElementRuntime(languageId, undefined, parent_element.enduser_name);
        }
        return Validator.findInCollection(languageId, runtimeStyleElements, name, (runtimeElementItem: RuntimeElement) => {
            if (!parent_element) {
                return true;
            }
            return (
                runtimeElementItem.belongs_to.length === 0 ||
                parent_runtime_element === undefined ||
                runtimeElementItem.belongs_to.some(
                    element => element.constructor.name === parent_runtime_element.constructor.name
                )
            );
        });
    }

    static getElementStyleStateRuntime(languageId: LanguageID, parent_element: AstLayoutElement | undefined, name: string): RuntimeElementStyleState | undefined {
        return Validator.findInCollection<RuntimeElementStyleState>(languageId, runtimeStyleStates, name, (runtimeStyleStateItem: RuntimeElementStyleState) => {
            if (!parent_element) {
                return true;
            }
            const value: string[] | undefined = runtimeStyleStateItem.getText(languageId);
            return value ? true : false;
        });
    }

    static getElementRuntime(languageId: LanguageID, parent_element: AstLayoutElement | undefined, name: string): RuntimeElement | undefined {
        return Validator.findElementRuntime(languageId, parent_element, name);
    }

    static getStyleElementRuntime(languageId: LanguageID, parent_element: AstLayoutElement | undefined, name: string): RuntimeElement | undefined {
        return Validator.findStyleElementRuntime(languageId, parent_element, name);
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

    static getElementAttributeStyle(languageId: LanguageID, name: string): RuntimeElementAttribute | undefined {
        return Validator.findInCollection(languageId, runtimeStyleAttributes, name);
    }

    static getElementAllAttributeRuntime(languageId: LanguageID, runtimeElement: RuntimeElement, attribute_name: string): RuntimeElementAttribute | undefined {
        const checks: (() => RuntimeElementAttribute | undefined)[] = [
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
    
    writeToFile(fileName: string): void {
        let fs: any;
        if (typeof window === "undefined") {
            fs = import('fs');
        } else {
            return;
        }

        try {
            fs.writeFileSync(fileName, this.ast.stringify(true), 'utf-8');
        } catch (error: unknown) {
            if (error instanceof Error) {
                this.pushError(validatorMessageRenderer(
                    this.getLanguageId(),
                    ValidatorMessageKeys.VALIDATOR_SAVE_OUTPUT_ERROR,
                    fileName,
                    error.message
                ));
            } else {
                this.pushError(validatorMessageRenderer(
                    this.getLanguageId(),
                    ValidatorMessageKeys.VALIDATOR_SAVE_OUTPUT_ERROR,
                    fileName,
                    "An unknown error occurred"
                ));
            }
        }
    }
}
