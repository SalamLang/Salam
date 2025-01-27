import { Generator } from "./../generator";
import { AstLayoutAttribute } from "./../../../parser/parse/ast/layout/attribute";
import { RuntimeElementAttributeOutputType } from "./../../../../runtime/element_attribute_output_type";

export function generateLayoutAttributeOutput(generator: Generator, attribute: AstLayoutAttribute): string {
    let output: string = '';

    // Prepare initial output
    if (attribute.generate_value === undefined) {
        return `""`;
    } else if (attribute.generate_value.includes(' ')) {
        output =  `"${attribute.generate_value}"`;
    } else {
        output = attribute.generate_value;
    }

    // Handle different output types
    switch (attribute.output_type) {
        case RuntimeElementAttributeOutputType.Normal:
            return output;
        case RuntimeElementAttributeOutputType.Url:
            return `url(${output})`;
        default:
            return output;
    }
};
