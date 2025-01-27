import { includeLayout } from './include';
import { Generator } from './../generator';
import { generateLayoutElement } from './element';
import { AstLayoutElement } from './../../../parser/parse/ast/layout/element';
import { generatorMessages } from './../../../../common/message/generator/generator';
import { messageRenderer, GeneratorMessageKeys } from './../../../../common/message/message';

export function generateLayoutNode(generator: Generator, element: AstLayoutElement): string {
    let result: string = '';
    // console.log("generateLayoutNode:", element);

    switch (element.generate_type) {
        case undefined: {
            result += "<!-- Unknown element type -->";
            result += generateLayoutElement(generator, element as AstLayoutElement);
            break;
        }
        case "RuntimeElementLayout": {
            result += "<!-- Layout element -->";
            result += generateLayoutElement(generator, element as AstLayoutElement);
            break;
        }
        case "RuntimeElementInclude": {
            const include_path: string | undefined = element.attributes.getByGenerateName("source")?.getValue();
            result += "<!-- Include: " + include_path + " -->";
            // console.log("Include path: ", include_path);
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
