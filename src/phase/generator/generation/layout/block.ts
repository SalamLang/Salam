import { includeLayout } from './include';
import { Generator } from './../generator';
import { generateLayoutElement } from './element';
import { AstLayoutBlock } from './../../../parser/parse/ast/layout/block';
import { AstLayoutElement } from './../../../parser/parse/ast/layout/element';
import { generatorMessages } from './../../../../common/message/generator/generator';
import { messageRenderer, GeneratorMessageKeys } from './../../../../common/message/message';

export function generateLayoutBlock(generator: Generator, element: AstLayoutElement, block: AstLayoutBlock): string {
    let result: string = "";

    for (const item of block.items) {
        // Handling custom elements
        if (element.generate_name === undefined) {
            switch (element.generate_type) {
                case "RuntimeElementLayout": {
                    console.log(element);
                    // result += generateLayoutElement(generator, element);
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
                    generator.pushError(messageRenderer(generatorMessages[generator.ast.language.id][GeneratorMessageKeys.GENERATOR_UNKNOWN_ELEMENT_TYPE], element.generate_type || "None"));
                    break;
                }
            }
        } else {
            result += generateLayoutElement(generator, item as AstLayoutElement);
        }
    }
    return result;
};
