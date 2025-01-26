import { Generator } from './../generator';
import { generateLayoutBlock } from './block';
import { AstLayoutElement } from './../../../parser/parse/ast/layout/element';

export function generateLayoutElement(generator: Generator, element: AstLayoutElement): void {
    if (element.generate_name === undefined) {
        switch (element.generate_type) {
            case "RuntimeElementLayout": {
                generator.writeLine(`<!doctype html>`);
                generator.writeLine(`<html>`);
                generator.indent();
                generator.writeLine(`<head>`);
                generator.indent();
                generator.writeLine(`<title>SALAM</title>`);
                generator.writeLine(`<meta charset="utf-8">`);
                generator.writeLine(`<meta name="viewport" content="width=device-width, initial-scale=1">`);
                generator.outdent();
                generator.writeLine(`</head>`);
                generator.writeLine(`<body>`);
                generator.indent();

                generateLayoutBlock(generator, element);

                generator.outdent();
                generator.writeLine(`</body>`);
                generator.outdent();
                generator.writeLine(`</html>`);
                return;
            }
        }
    }

    generator.writeLine(`<${element.generate_name}>`);
    generator.indent();
    generator.outdent();
    generator.writeLine(`</${element.generate_name}>`);
};
