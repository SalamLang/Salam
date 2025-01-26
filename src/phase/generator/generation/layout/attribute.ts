import { Generator } from "./../generator";
import { AstLayoutAttribute } from "../../../parser/parse/ast/layout/attribute";

export function generateLayoutAttribute(generator: Generator, attribute: AstLayoutAttribute): void {
    generator.write(attribute.generate_name + '=');

    if (attribute.generate_value === undefined) {
        attribute.generate_value = attribute.value.getString();
    }

    if (attribute.generate_value === undefined) {
        generator.write('""');
    } else if (attribute.generate_value.includes(' ')) {
        generator.write('"' + attribute.generate_value + '"');
    } else {
        generator.write(attribute.generate_value);
    }
};
