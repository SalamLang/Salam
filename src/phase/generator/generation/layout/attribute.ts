import { Generator } from "./../generator";
import { AstLayoutAttribute } from "./../../../parser/parse/ast/layout/attribute";
import { AstLayoutAttributeType } from "./../../../parser/parse/ast/layout/attribute_type";
import { generateLayoutAttributeOutput } from "./attribute_output";

export function generateLayoutAttribute(generator: Generator, attribute: AstLayoutAttribute): string {
    let result: string = "";

    const isStyle: boolean = attribute.kind === AstLayoutAttributeType.Style;

    if (attribute.generate_name === undefined) {
        return "";
    }
    
    result += attribute.generate_name + (isStyle ? ':' : '=');

    if (attribute.generate_value === undefined) {
        attribute.generate_value = attribute.value.getString();
    }

    result += generateLayoutAttributeOutput(generator, attribute);

    return result;
};
