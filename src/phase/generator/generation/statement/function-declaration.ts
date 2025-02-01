import { generateBlock } from './block';
import { Generator } from './../generator';
import { generateFunctionParameters } from './function-parameters';
import { AstFunctionDeclaration } from './../../../parser/parse/ast/statement/function_declaration';

export function generateFunctionDeclaration(generator: Generator, func: AstFunctionDeclaration): string {
    let result: string = "";
    let sign: string = "";

    sign += "void ";
    sign += func.name;
    sign += "(";
    if (func.parameters) {
        sign += generateFunctionParameters(generator, func.parameters);
    }
    sign += ")";

    generator.pushSignFunction(sign + ";");

    result += sign;
    result += " ";
    result += generateBlock(generator, func.body);
    return result;
};
