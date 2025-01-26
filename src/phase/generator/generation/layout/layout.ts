import { Generator } from './../generator';
import { generateLayoutElement } from './element';
import { AstLayout } from './../../../parser/parse/ast/layout/layout';

export function generateLayout(generator: Generator, layout: AstLayout): string {
    return generateLayoutElement(generator, layout.root);
};
