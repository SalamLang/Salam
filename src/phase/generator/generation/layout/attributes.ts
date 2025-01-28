import { Generator } from "./../generator";
import { generateLayoutAttribute } from "./attribute";
import { AstLayoutElement } from "./../../../parser/parse/ast/layout/element";
import { AstLayoutAttributes } from "./../../../parser/parse/ast/layout/attributes";

export function generateLayoutAttributes(generator: Generator, element: AstLayoutElement, attributes: AstLayoutAttributes): string {
    let htmlResult: string = "";
    let styleResult: string = "";

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

    console.log(styleResult);
    console.log(element);

    if (styleResult.length > 0 && element.has_style && element.built_in_selector !== undefined) {
        const element_selector: string = element.built_in_selector;
        generator.pushStyle(`${element_selector} {${styleResult}}`);
    }

    return htmlResult;
    // return styleResult
    //     ? `${htmlResult} style="${styleResult.replace(/"/g, '\\"')}"`.trim()
    //     : htmlResult;
};
