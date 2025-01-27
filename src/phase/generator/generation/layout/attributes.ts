import { Generator } from "./../generator";
import { generateLayoutAttribute } from "./attribute";
import { AstLayoutAttributes } from "./../../../parser/parse/ast/layout/attributes";

export function generateLayoutAttributes(generator: Generator, attributes: AstLayoutAttributes): string {
    let style_result: string = "";
    let html_result: string = "";

    for (const attribute of attributes.items) {
        if (attribute.isStyle()) {
            if (style_result.length > 0) {
                style_result += ";";
            }
            style_result += generateLayoutAttribute(generator, attribute);
        } else {
            if (html_result.length > 0) {
                html_result += " ";
            }
            html_result += generateLayoutAttribute(generator, attribute);
        }
    }

    let result: string = "";
    if (html_result.length > 0) {
        result += html_result;
    }
    if (style_result.length > 0) {
        if (result.length > 0) {
            result += " ";
        }
        result += `style="${style_result.replace(/"/g, '\\"')}"`;
    }
    return result;
};
