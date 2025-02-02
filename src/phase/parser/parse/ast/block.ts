import { AstNode } from './node';
import { stringify } from './../../../../serializer';
import { SymbolTable } from '../../../validator/validation/symbol-table';

export class AstBlock extends AstNode {
	children: AstNode[] = [];
	symbol_table: SymbolTable;
	checkVariables: boolean;
	parent_block: AstBlock | undefined;

	constructor() {
		super("Block");
		this.symbol_table = new SymbolTable();
		this.checkVariables = true;
		this.parent_block = undefined;
	}

	setParentBlock(parent_block: AstBlock): void {
		this.parent_block = parent_block;
	}

	addChild(node: AstNode): void {
		this.children.push(node);
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
