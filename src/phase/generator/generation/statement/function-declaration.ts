import { generateBlock } from './block';
import { Generator } from './../generator';
import { AstFunctionDeclaration } from './../../../parser/parse/ast/statement/function_declaration';

export function generateFunctionDeclaration(generator: Generator, func: AstFunctionDeclaration): string {
    let result: string = "";
    result += "void ";
    result += func.name;
    result += "(";
    // if (func.parameters) {
    //     result += func.parameters.map(param => {
    //         return param.type + " " + param.name;
    //     }).join(", ");
    // }
    result += ") ";
    result += generateBlock(generator, func.body);
    return result;
};
