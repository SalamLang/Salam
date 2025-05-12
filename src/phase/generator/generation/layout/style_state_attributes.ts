import { Generator } from "./../generator";
import { generateLayoutAttribute } from "./attribute";
import { AstLayoutAttributes } from "./../../../parser/parse/ast/layout/attributes";

export function generateLayoutStyleStateAttributes(generator: Generator, attributes: AstLayoutAttributes): string {
    let styleResult: string = "";

    for (const attribute of attributes.items) {
        if (attribute.isStyle()) {
            const generatedAttribute: string = generateLayoutAttribute(generator, attribute);
            if (generatedAttribute.length > 0) {
                styleResult += styleResult.length > 0 ? `;${generatedAttribute}` : generatedAttribute;
            }
        }
    }

    return styleResult;
};
