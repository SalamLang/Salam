import { Generator } from './../generator';
import { generateLayoutBlock } from './block';
import { AstLayoutElement } from './../../../parser/parse/ast/layout/element';
import { generateLayoutAttribute } from './attribute';

export function generateLayoutElement(generator: Generator, element: AstLayoutElement): void {
    if (element.generate_name === undefined) {
        switch (element.generate_type) {
            case "RuntimeElementLayout": {
                generator.writeIndentLine(`<!doctype html>`);
                generator.writeIndentLine(`<html>`);
                generator.indent();
                generator.writeIndentLine(`<head>`);
                generator.indent();
                generator.writeIndentLine(`<title>SALAM</title>`);
                generator.writeIndentLine(`<meta charset="utf-8">`);
                generator.writeIndentLine(`<meta name="viewport" content="width=device-width, initial-scale=1">`);
                generator.outdent();
                generator.writeIndentLine(`</head>`);
                generator.writeIndentLine(`<body>`);
                generator.indent();

                generateLayoutBlock(generator, element.block);

                generator.outdent();
                generator.writeIndentLine(`</body>`);
                generator.outdent();
                generator.writeIndentLine(`</html>`);
                return;
            }
        }
    }

    generator.writeIndentNoLine(`<${element.generate_name}`);
    if (element.globalAttributes.items.length > 0) {
        generator.write(` `);
    }
    for (const attribute of element.globalAttributes.items) {
        generateLayoutAttribute(generator, attribute);
    }
    generator.write(`>`);

    if (element.block.length > 0) {
        generator.writeLine("");
        generator.indent();
        generateLayoutBlock(generator, element.block);
        generator.outdent();
        generator.writeIndentLine(`</${element.generate_name}>`);
    } else {
        generator.writeLine(`</${element.generate_name}>`);
    }
};
