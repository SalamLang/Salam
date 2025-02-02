import { AstType } from "../../parser/parse/ast/expression/type";

export class SymbolTable {
    private table: Map<string, AstType>;

    constructor() {
        this.table = new Map<string, AstType>();
    }

    addSymbol(name: string, type: AstType): void {
        this.table.set(name, type);
    }

    getSymbol(name: string): AstType | undefined {
        return this.table.get(name);
    }

    hasSymbol(name: string): boolean {
        return this.table.has(name);
    }

    removeSymbol(name: string): boolean {
        return this.table.delete(name);
    }
}
