import { Generator } from "./../generator";
import { AstLayoutAttribute } from "./../../../parser/parse/ast/layout/attribute";
import { RuntimeElementAttributeOutputType } from "./../../../../runtime/element_attribute_output_type";

export function generateLayoutAttributeOutput(generator: Generator, attribute: AstLayoutAttribute): string {
    if (attribute.generate_value === undefined) {
        return "";
    }
    const output: string = attribute.generate_value;

    // Handle different output types
    switch (attribute.output_type) {
        case RuntimeElementAttributeOutputType.Normal:
            if (output.includes(" ") || /[^a-zA-Z0-9]/.test(output)) {
                return `"${output}"`;
            }
            return output;

        case RuntimeElementAttributeOutputType.UrlWithFont:
            const extenstionMappe: { [key: string]: string } = {
                ".woff": "woff",
                ".woff2": "woff2",
                ".ttf": "truetype",
                ".otf": "opentype",
                ".eot": "embedded-opentype",
                ".svg": "svg"
            };
            const extenstion: string = output.split('.').pop() || "";
            const format: string = extenstionMappe["." + extenstion];
            return `url('${attribute.getValue().replace("'", "")}') format('${format}')`;

        case RuntimeElementAttributeOutputType.Url:
            return `url(${output})`;
    }
    return "";
};
