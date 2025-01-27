import { Generator } from "./../generator";
import { generateLayoutAttributeOutput } from "./attribute_output";
import { AstLayoutAttribute } from "./../../../parser/parse/ast/layout/attribute";

export function generateLayoutAttribute(generator: Generator, attribute: AstLayoutAttribute): string {
    let result: string = "";

    if (attribute.generate_name === undefined) {
        return "";
    }
    
    // Append the name
    result += attribute.generate_name + (attribute.isStyle() ? ':' : '=');

    // Generate the value
    if (attribute.generate_value === undefined) {
        attribute.generate_value = attribute.value.getString();
    }

    // Append the value
    result += generateLayoutAttributeOutput(generator, attribute);

    return result;
};
