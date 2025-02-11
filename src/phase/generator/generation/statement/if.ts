import { Generator } from '../generator';
import { AstStatementIf } from '../../../parser/parse/ast/statement/if';

export function generateStatementIf(generator: Generator, stmt: AstStatementIf): string {
    let result: string = '';

    result += `if (true) {}\n`;
    
    return result;
};
