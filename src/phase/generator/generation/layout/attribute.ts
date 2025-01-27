import { Generator } from "./../generator";
import { AstLayoutAttribute } from "./../../../parser/parse/ast/layout/attribute";

export function generateLayoutAttribute(generator: Generator, attribute: AstLayoutAttribute): string {
    let result: string = "";

    if (attribute.generate_name === undefined) {
        return "";
    }
    
    result += attribute.generate_name + '=';

    if (attribute.generate_value === undefined) {
        attribute.generate_value = attribute.value.getString();
    }

    if (attribute.generate_value === undefined) {
        result += `""`;
    } else if (attribute.generate_value.includes(' ')) {
        result += `"${attribute.generate_value}"`;
    } else {
        result += attribute.generate_value;
    }

    return result;
};
