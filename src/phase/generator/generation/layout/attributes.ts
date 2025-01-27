import { Generator } from "./../generator";
import { generateLayoutAttribute } from "./attribute";
import { AstLayoutAttributes } from "./../../../parser/parse/ast/layout/attributes";

export function generateLayoutAttributes(generator: Generator, attributes: AstLayoutAttributes): string {
    let styleResult: string = "";
    let htmlResult: string = "";

    for (const attribute of attributes.items) {
        const generatedAttribute = generateLayoutAttribute(generator, attribute);
        if (generatedAttribute.length > 0) {
            if (attribute.isStyle()) {
                styleResult += styleResult.length > 0 ? `;${generatedAttribute}` : generatedAttribute;
            } else {
                htmlResult += htmlResult.length > 0 ? ` ${generatedAttribute}` : generatedAttribute;
            }
        }
    }

    return styleResult
        ? `${htmlResult} style="${styleResult.replace(/"/g, '\\"')}"`.trim()
        : htmlResult;
};
