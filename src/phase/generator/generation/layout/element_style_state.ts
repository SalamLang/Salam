import { Generator } from './../generator';
import { AstLayoutElement } from './../../../parser/parse/ast/layout/element';
import { generateLayoutStyleStateAttributes } from './style_state_attributes';

export function generateLayoutElementStyleState(generator: Generator, element_style_state: AstLayoutElement): string {
    let result: string = "";
    const attributes_str: string = generateLayoutStyleStateAttributes(generator, element_style_state.styles);

    const element_selector: string | undefined = element_style_state.built_in_selector;
    if (element_selector) {
        result += `${element_selector}:${element_style_state.generate_name} {`;
        result += attributes_str;
        result += "}";
    }

    return result;
};
