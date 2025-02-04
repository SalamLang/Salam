import { Generator } from './generator';
import { generateLayout } from './layout/layout';
import { generateFunctionDeclaration } from './statement/function_declaration';

export function generate(generator: Generator): void {
    if (generator.ast.layout) {
        generator.source += generateLayout(generator, generator.ast.layout);
    }
    for (const func of generator.ast.functions) {
        const func_str: string = generateFunctionDeclaration(generator, func);
        if (func_str.length > 0) {
            generator.pushFunction(func_str);
        }
    }
};
