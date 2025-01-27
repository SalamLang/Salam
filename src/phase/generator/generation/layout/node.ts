import { includeLayout } from './include';
import { Generator } from './../generator';
import { generateLayoutElement } from './element';
import { AstLayoutElement } from './../../../parser/parse/ast/layout/element';
import { generatorMessages } from './../../../../common/message/generator/generator';
import { messageRenderer, GeneratorMessageKeys } from './../../../../common/message/message';

export function generateLayoutNode(generator: Generator, element: AstLayoutElement): string {
    let result: string = '';
    // console.log("generateLayoutNode:", element.type);

    switch (element.generate_type) {
        case undefined: {
            console.log("Unknown element:", element);
            result += "<!-- Unknown element -->";
            result += generateLayoutElement(generator, element as AstLayoutElement);
            break;
        }
        case "RuntimeElementLayout": {
            result += generateLayoutElement(generator, element as AstLayoutElement);
            break;
        }
        case "RuntimeElementInclude": {
            const include_path: string | undefined = element.attributes.getByGenerateName("source")?.getValue();
            if (include_path) {
                result += includeLayout(generator, include_path, []);
            }
            break;
        }
        default: {
            console.log(element.generate_type, element);
            generator.pushError(messageRenderer(generatorMessages[generator.ast.language.id][GeneratorMessageKeys.GENERATOR_UNKNOWN_ELEMENT_TYPE], element.generate_type || "None"));
            break;
        }
    }

    return result;
};
