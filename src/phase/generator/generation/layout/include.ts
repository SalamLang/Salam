import fs from 'fs';
import path from 'path';

import { generateLayoutNode } from './node';
import { lex } from './../../../lexer/lex/lex';
import { Lexer } from './../../../lexer/lex/lexer';
import { parse } from './../../../parser/parse/parse';
import { Parser } from './../../../parser/parse/parser';
import { validate } from './../../../validator/validation/validate';
import { Validator } from './../../../validator/validation/validator';
import { Generator } from './../../../generator/generation/generator';
import { generatorMessageRenderer } from './../../../../common/message/message';
import { GeneratorMessageKeys } from './../../../../common/message/generator/generator';

export function includeLayout(generator: Generator, filePath: string, params: string[]): string {
    if (! filePath) {
        generator.pushError(generatorMessageRenderer(generator.ast.language.id, GeneratorMessageKeys.GENERATOR_INCLUDE_FILE_PATH_NOT_PROVIDED));
    } else if (! fs.existsSync(filePath)) {
        generator.pushError(generatorMessageRenderer(generator.ast.language.id, GeneratorMessageKeys.GENERATOR_INCLUDE_FILE_NOT_FOUND, filePath));
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
        
        if (_parser.ast.errors.length > 0) {
            _parser.ast.errors.forEach((error: string) => {
                console.error(error);
                process.exit(1);
            });
        }
        
        if (_validator.errors.length > 0) {
            _validator.errors.forEach((error: string) => {
                console.error(error);
                process.exit(1);
            });
        }

        const _generator: Generator = new Generator(_validator.ast);
        if (_generator.errors.length > 0) {
            _generator.errors.forEach((error: string) => {
                console.error(error);
                process.exit(1);
            });
        }
        
        if (_generator.ast.layout !== undefined) {
            return generateLayoutNode(generator, _generator.ast.layout.root);
        }
    }
    return "";
};