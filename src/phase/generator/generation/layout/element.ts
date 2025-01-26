import { Generator } from './../generator';
import { generateLayoutBlock } from './block';
import { AstLayoutElement } from './../../../parser/parse/ast/layout/element';
import { generateLayoutAttribute } from './attribute';

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

    generator.writeNoLine(`<${element.generate_name}`);
    if (element.globalAttributes.items.length > 0) {
        generator.write(` `);
    }
    for (const attribute of element.globalAttributes.items) {
        generateLayoutAttribute(generator, attribute);
    }
    generator.write(`>`);

    if (element.children.length > 0) {
        generator.write("\n");
        generator.indent();
        for (const child of element.children) {
            generateLayoutElement(generator, child as AstLayoutElement);
        }
        generator.outdent();
        generator.writeLine(`</${element.generate_name}>`);
    } else {
        generator.write(`</${element.generate_name}>\n`);
    }
};
