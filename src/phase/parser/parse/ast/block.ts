import { AstNode } from './node';
import { stringify } from './../../../../serializer';
import { SymbolTable } from '../../../validator/validation/symbol-table';

export class AstBlock extends AstNode {
	children: AstNode[] = [];
	table: SymbolTable;

	constructor() {
		super("Block");
		this.table = new SymbolTable();
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
		};
		return stringify(obj, wantsJson);
	}
}
