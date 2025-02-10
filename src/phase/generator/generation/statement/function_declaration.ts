import { generateBlock } from './block';
import { Generator } from '../generator';
import { generateFunctionArguments } from './function_arguments';
import { AstFunctionDeclaration } from '../../../parser/parse/ast/function/function_declaration';
import { generateType } from '../expression/type';

export function generateFunctionDeclaration(generator: Generator, func: AstFunctionDeclaration): string {
    let result: string = "";
    let sign: string = "";

    sign += generateType(generator, func.return_type)
    sign += " ";
    sign += func.name;
    sign += "(";
    sign += generateFunctionArguments(generator, func.args);
    sign += ")";

    generator.pushSignFunction(sign + ";");

    result += sign;
    result += " ";
    result += generateBlock(generator, func.body);
    return result;
};
