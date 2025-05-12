import { Generator } from '../generator';
import { AstStatementDefer } from '../../../parser/parse/ast/statement/defer';

export function generateStatementDefer(generator: Generator, stmt: AstStatementDefer): string {
    let result: string = '';

    result += `// defer return\n`;
    
    return result;
};
