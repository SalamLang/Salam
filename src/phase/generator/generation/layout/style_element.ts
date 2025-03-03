import { Generator } from './../generator';
import { generateLayoutBlock } from './block';
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
        case "RuntimeStyleElementMedia": {
            let style: string = "@media ";
            const attribute_condition: string | undefined = element.attributes.getByGenerateName("condition")?.getValue();
            style += attribute_condition;
            style += " {";
            style += element.built_in_selector;
            style += " {";
            const attributes_str: string = generateLayoutStyleStateAttributes(generator, element.styles);
            if (attributes_str !== "") {
                style += attributes_str;
            }
            style += "}";
            style += "}";
            generator.pushMediaStyle(style);
            return "";
        }
        case "RuntimeStyleElementIcon": {
            let head: string = "";

            const attribute_type: string | undefined = element.attributes.getByGenerateName("type")?.getValue();
            const attribute_source: string | undefined = element.attributes.getByGenerateName("source")?.getValue();
            const attribute_size: string | undefined = element.attributes.getByGenerateName("size")?.getValue();
            const attribute_format: string | undefined = element.attributes.getByGenerateName("format")?.getValue() || "image/x-icon";

            if (attribute_type === undefined || attribute_type === "normal") {
                head += `<link rel="icon"`;
                head += ` type="${attribute_format}"`;
                head += ` href="${attribute_source}">`;
                generator.pushHead(head);
            } else if (attribute_type === "apple") {
                head += `<link rel="apple-touch-icon"`
                head += ` sizes="${attribute_size}"`;
                head += ` href="${attribute_source}">`;
                generator.pushHead(head);
            }
            return "";
        }
        case "RuntimeStyleElementViewport": {
            let head: string = "";

            const attribute_content: string | undefined = element.attributes.getByGenerateName("content")?.getValue();
            if (attribute_content !== undefined) {
                head += `<meta name="viewport" content="${attribute_content}">`;
                generator.pushHead(head);
            }
            return "";
        }
        default: {
            generator.pushError("Error: cannot handle " + element.generate_type);
        }
    }

    return "";
};
