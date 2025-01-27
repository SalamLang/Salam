import fs from 'fs';
import path from 'path';

import { generateLayoutBlock } from './block';
import { lex } from './../../../lexer/lex/lex';
import { Lexer } from './../../../lexer/lex/lexer';
import { parse } from './../../../parser/parse/parse';
import { Parser } from './../../../parser/parse/parser';
import { validate } from './../../../validator/validation/validate';
// import { generate } from './../../../generator/generation/generate';
import { Validator } from './../../../validator/validation/validator';
import { Generator } from './../../../generator/generation/generator';

export function includeLayout(generator: Generator, filePath: string, params: string[]): string {
    console.log(filePath);

    try {
        if (! filePath) {
            generator.pushError("File path not provided.");
        } else if (! fs.existsSync(filePath)) {
            generator.pushError(`File not found - "${filePath}"`);
        } else {
            const fileName: string = fs.realpathSync(filePath);
            const absoluteDirPath: string = fs.realpathSync(path.dirname(filePath));
            const source: string = fs.readFileSync(fileName, 'utf8');
    
            const _lexer: Lexer = new Lexer(source, generator.ast.language, fileName, absoluteDirPath);
            lex(_lexer);
            const _parser: Parser = new Parser(_lexer);
            parse(_parser);
            const _validator: Validator = new Validator(_parser.ast);
            validate(_validator);
            const _generator: Generator = new Generator(_validator.ast);
            if (_generator.ast.layout !== undefined) {
                return generateLayoutBlock(generator, _generator.ast.layout.root, _generator.ast.layout.root.block);
            }
        }
    } catch (error: any) {
        if (error.code === 'ENOENT') {
            generator.pushError(`File not found - "${filePath}"`);
        } else {
            generator.pushError(`File read error: ${error.message}`);
        }
    }
    return "";
};
