import { Generator } from './../generator';
import { generateLayoutElement } from './element';
import { AstLayout } from './../../../parser/parse/ast/layout/layout';
import { AstLayoutElement } from './../../../parser/parse/ast/layout/element';

export function generateLayout(generator: Generator, layout: AstLayout): void {
    const element = new AstLayoutElement(layout.enduser_name);
    layout.applyToElement(element);

    generateLayoutElement(generator, element);
};
 