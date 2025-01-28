import fs from 'fs';
import path from 'path';

import { generateLayoutNode } from './node';
import { lex } from './../../../lexer/lex/lex';
import { Lexer } from './../../../lexer/lex/lexer';
import { parse } from './../../../parser/parse/parse';
import { Parser } from './../../../parser/parse/parser';
import { checkError } from './../../../../common/cli/check-error';
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
        checkError(_parser, undefined, undefined);

        const _validator: Validator = new Validator(_parser.ast);
        validate(_validator);
        checkError(_parser, _validator, undefined);

        const _generator: Generator = new Generator(_validator.ast);
        if (_generator.ast.layout !== undefined) {
            const result: string = generateLayoutNode(generator, _generator.ast.layout.root);
            checkError(_parser, _validator, _generator);
            return generator.buffer(result);
        }
    }
    return "";
};