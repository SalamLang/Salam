import { Generator } from './generator';
import { generateLayout } from './layout/layout';

export function generate(generator: Generator): void {
    if (generator.ast.layout) {
        generateLayout(generator, generator.ast.layout);
    }
};
