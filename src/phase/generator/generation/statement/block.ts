import { Generator } from './../generator';
import { generateStatement } from './statement';
import { AstBlock } from './../../../parser/parse/ast/block';

export function generateBlock(generator: Generator, block: AstBlock): string {
    let result: string = "{\n";
    for (const statement of block.children) {
        result += generateStatement(generator, statement);
    }
    result += "}\n";
    return result;
};
