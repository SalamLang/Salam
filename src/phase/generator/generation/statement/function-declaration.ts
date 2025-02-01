import { generateBlock } from './block';
import { Generator } from './../generator';
import { AstFunctionDeclaration } from './../../../parser/parse/ast/statement/function_declaration';

export function generateFunctionDeclaration(generator: Generator, func: AstFunctionDeclaration): string {
    let result: string = "";
    let sign: string = "";

    sign += "void ";
    sign += func.name;
    sign += "(";
    // if (func.parameters) {
    //     sign += func.parameters.map(param => {
    //         return param.type + " " + param.name;
    //     }).join(", ");
    // }
    sign += ")";

    generator.pushSignFunction(sign + ";");

    result += sign;
    result += " ";
    result += generateBlock(generator, func.body);
    return result;
};
