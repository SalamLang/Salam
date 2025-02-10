import fs from 'fs';
import path from 'path';
import fetch from 'sync-fetch';

import { generateLayoutNode } from './node';
import { lex } from './../../../lexer/lex/lex';
import { Lexer } from './../../../lexer/lex/lexer';
import { SALAM_VERSION } from './../../../../base';
import { parse } from './../../../parser/parse/parse';
import { Parser } from './../../../parser/parse/parser';
import { isUrl } from './../../../validator/validation/is-url';
import { checkError } from './../../../../common/cli/check-error';
import { validate } from './../../../validator/validation/validate';
import { Validator } from './../../../validator/validation/validator';
import { Generator } from './../../../generator/generation/generator';
import { generatorMessageRenderer } from './../../../../common/message/message';
import { GeneratorMessageKeys } from './../../../../common/message/generator/generator';

export function includeLayoutString(generator: Generator, source: string, fileName: string, absoluteDirPath: string, params: string[]) {
    const _lexer = new Lexer(source, generator.ast.language, fileName, absoluteDirPath);
    lex(_lexer);

    const _parser = new Parser(_lexer);
    parse(_parser);
    checkError(_parser, undefined, undefined);

    const _validator = new Validator(_parser.ast);
    validate(_validator);
    checkError(_parser, _validator, undefined);

    const _generator = new Generator(_validator.ast, _validator.extendedFunctions, _validator.extendedVariables, _validator.packages);
    if (_generator.ast.layout !== undefined) {
        _generator.ast.layout.root.generate_name = "div";
        const result = generateLayoutNode(generator, _generator.ast.layout.root);
        checkError(_parser, _validator, _generator);
        return generator.buffer(result);
    }
    return "";
}

function fetchUrlSync(url: string): string {
    try {
        const res = fetch(url, {
            headers: { "User-Agent": `Salam/${SALAM_VERSION}` }
        });
        return res.text();
    } catch (error) {
        return "";
    }
}

export function includeLayout(generator: Generator, filePath: string, params: string[]): string {
    if (isUrl(filePath)) {
        try {
            const source = fetchUrlSync(filePath);
            if (!source) {
                generator.pushError(
                    generatorMessageRenderer(
                        generator.getLanguageId(),
                        GeneratorMessageKeys.GENERATOR_INCLUDE_HTTP_ERROR,
                        filePath
                    )
                );
                return "";
            }
            return includeLayoutString(generator, source, "online.salam", process.cwd(), params);
        } catch (error) {
            generator.pushError(
                generatorMessageRenderer(
                    generator.getLanguageId(),
                    GeneratorMessageKeys.GENERATOR_INCLUDE_HTTP_ERROR,
                    filePath
                )
            );
        }
    } else {
        if (!filePath) {
            generator.pushError(
                generatorMessageRenderer(
                    generator.getLanguageId(),
                    GeneratorMessageKeys.GENERATOR_INCLUDE_FILE_PATH_NOT_PROVIDED
                )
            );
        } else if (!fs.existsSync(filePath)) {
            generator.pushError(
                generatorMessageRenderer(
                    generator.getLanguageId(),
                    GeneratorMessageKeys.GENERATOR_INCLUDE_FILE_NOT_FOUND,
                    filePath
                )
            );
        } else {
            try {
                const fileName = fs.realpathSync(filePath);
                const absoluteDirPath = fs.realpathSync(path.dirname(filePath));
                const source = fs.readFileSync(fileName, "utf8");
                return includeLayoutString(generator, source, fileName, absoluteDirPath, params);
            } catch (error) {
                generator.pushError(
                    generatorMessageRenderer(
                        generator.getLanguageId(),
                        GeneratorMessageKeys.GENERATOR_INCLUDE_FILE_READ_ERROR,
                        filePath
                    )
                );
            }
        }
    }
    return "";
}
