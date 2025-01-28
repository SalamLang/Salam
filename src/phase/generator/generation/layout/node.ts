import { includeLayout } from './include';
import { Generator } from './../generator';
import { generateLayoutElement } from './element';
import { AstLayoutElement } from './../../../parser/parse/ast/layout/element';
import { generatorMessageRenderer } from './../../../../common/message/message';
import { GeneratorMessageKeys } from './../../../../common/message/generator/generator';

export function generateLayoutNode(generator: Generator, element: AstLayoutElement): string {
    let result: string = '';
    switch (element.generate_type) {
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
            generator.pushError(generatorMessageRenderer(generator.ast.language.id, GeneratorMessageKeys.GENERATOR_UNKNOWN_ELEMENT_TYPE, element.generate_type || "None"));
            break;
        }
    }

    return result;
};
