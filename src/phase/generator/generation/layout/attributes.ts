import { Generator } from "./../generator";
import { generateLayoutAttribute } from "./attribute";
import { AstLayoutAttributes } from "../../../parser/parse/ast/layout/attributes";

export function generateLayoutAttributes(generator: Generator, attributes: AstLayoutAttributes): string {
    let result: string = "";

    for (const attribute of attributes.items) {
        result += generateLayoutAttribute(generator, attribute);
    }

    if (result.length > 0) {
        result = " " + result;
    }

    return result;
};
