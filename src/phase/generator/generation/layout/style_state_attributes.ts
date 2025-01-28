import { Generator } from "./../generator";
import { generateLayoutAttribute } from "./attribute";
import { AstLayoutAttributes } from "./../../../parser/parse/ast/layout/attributes";

export function generateLayoutStyleStateAttributes(generator: Generator, attributes: AstLayoutAttributes): string {
    let styleResult: string = "";

    for (const attribute of attributes.items) {
        const generatedAttribute = generateLayoutAttribute(generator, attribute);
        if (generatedAttribute.length > 0) {
            if (attribute.isStyle()) {
                styleResult += styleResult.length > 0 ? `;${generatedAttribute}` : generatedAttribute;
            }
        }
    }

    return styleResult;
};
