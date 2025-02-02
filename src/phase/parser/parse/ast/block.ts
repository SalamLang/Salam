import { AstNode } from './node';
import { stringify } from './../../../../serializer';
import { SymbolTable } from '../../../validator/validation/symbol-table';
import { AstType } from './expression/type';

export class AstBlock extends AstNode {
	children: AstNode[] = [];
	symbol_table: SymbolTable;
	parent_block: AstBlock | undefined;

	constructor() {
		super("Block");
		this.symbol_table = new SymbolTable();
		this.parent_block = undefined;
	}

	setParentBlock(parent_block: AstBlock): void {
		this.parent_block = parent_block;
	}

	addChild(node: AstNode): void {
		this.children.push(node);
	}

	lookUp(name: string): AstType | undefined {
		let currentBlock: AstBlock | undefined = this;

		while (currentBlock) {
			const symbol = currentBlock.symbol_table.getSymbol(name);
			if (symbol !== undefined) {
				return symbol;
			}
			currentBlock = currentBlock.parent_block;
		}
	
		return undefined;
	}

	print(): void {
		console.log(this.stringify());
	}

	stringify(wantsJson: boolean = true): string | object {
		const obj: object = {
			children: this.children,
			symbol_table: this.symbol_table.stringify(false),
		};
		return stringify(obj, wantsJson);
	}
}
