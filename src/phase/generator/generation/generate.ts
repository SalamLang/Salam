import { Generator } from './generator';
import { generateLayout } from './layout/layout';
import { generateFunctionDeclaration } from './statement/function-declaration';

export function generate(generator: Generator): void {
    if (generator.ast.layout) {
        generator.source += generateLayout(generator, generator.ast.layout);
    }
    console.log(generator.ast);
    for (const func of generator.ast.functions) {
        console.log(func);
        generator.source_c += generateFunctionDeclaration(generator, func);
    }
};
