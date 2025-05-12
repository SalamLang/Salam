import { Generator } from './../generator';
import { generateLayoutBlock } from './block';
import { AstLayoutBlock } from '../../../parser/parse/ast/layout/block';
import { AstLayoutElement } from './../../../parser/parse/ast/layout/element';
import { generateLayoutStyleStateAttributes } from './style_state_attributes';

export function generateLayoutElementStyleState(generator: Generator, block: AstLayoutBlock, element_style_state: AstLayoutElement): string {
    let result: string = "";
    const attributes_str: string = generateLayoutStyleStateAttributes(generator, element_style_state.styles);

    const element_selector: string | undefined = element_style_state.built_in_selector;
    if (element_selector) {
        result += `${element_selector}`;
        result += ":";
        result += `${element_style_state.generate_name} {`;
        result += attributes_str;
        result += "}";
    }

    generateLayoutBlock(generator, element_style_state, element_style_state.block);

    return result;
};
