import { Generator } from './../generator';
import { AstLayoutElement } from './../../../parser/parse/ast/layout/element';
import { generateLayoutStyleStateAttributes } from './style_state_attributes';

export function generateLayoutStyleElement(generator: Generator, element: AstLayoutElement): string {
    switch (element.generate_type) {
        case "RuntimeStyleElementFont": {
            let style: string = "@font-face {";
            style += generateLayoutStyleStateAttributes(generator, element.attributes);
            style += "}";
            generator.pushStyleTop(style);
            return "";
        }
    }

    return "";
};
