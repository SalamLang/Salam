import { Generator } from './generator';
import { generateLayout } from './layout/layout';

export function generate(generator: Generator): void {
    generator.source = '';

    if (generator.ast.layout) {
        generator.source += generateLayout(generator, generator.ast.layout);
    }
};
