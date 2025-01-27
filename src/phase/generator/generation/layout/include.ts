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
import { generatorMessages } from './../../../../common/message/generator/generator';
import { messageRenderer, GeneratorMessageKeys } from './../../../../common/message/message';

export function includeLayout(generator: Generator, filePath: string, params: string[]): string {
    try {
        if (! filePath) {
            generator.pushError(messageRenderer(generatorMessages[generator.ast.language.id][GeneratorMessageKeys.GENERATOR_INCLUDE_FILE_PATH_NOT_PROVIDED]));
        } else if (! fs.existsSync(filePath)) {
            generator.pushError(messageRenderer(generatorMessages[generator.ast.language.id][GeneratorMessageKeys.GENERATOR_INCLUDE_FILE_NOT_FOUND], filePath));
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
                console.log(_generator.ast.layout.root);
                return generateLayoutBlock(generator, _generator.ast.layout.root, _generator.ast.layout.root.block);
            }
        }
    } catch (error: any) {
        if (error.code === 'ENOENT') {
            generator.pushError(messageRenderer(generatorMessages[generator.ast.language.id][GeneratorMessageKeys.GENERATOR_INCLUDE_FILE_NOT_FOUND], filePath));
        } else {
            generator.pushError(messageRenderer(generatorMessages[generator.ast.language.id][GeneratorMessageKeys.GENERATOR_INCLUDE_FILE_READ_ERROR], error.message));
        }
    }
    return "";
};
