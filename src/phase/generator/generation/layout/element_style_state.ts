import { Generator } from './../generator';
import { AstLayoutElement } from './../../../parser/parse/ast/layout/element';
import { generateLayoutStyleStateAttributes } from './style_state_attributes';

export function generateLayoutElementStyleState(generator: Generator, element: AstLayoutElement): string {
    let result: string = "";
    const attributes_str: string = generateLayoutStyleStateAttributes(generator, element.attributes);

    if (attributes_str.length === 0) {
        return "Warning: Empty style state";
    }

    result += element.generate_name + " {";
    result += attributes_str;
    result += "}";

    return result;
};
