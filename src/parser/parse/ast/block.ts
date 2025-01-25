import { AstNode } from './node';

export class AstBlock extends AstNode {
	children: AstNode[] = [];

	constructor() {
		super("Block");
	}

	addChild(node: AstNode): void {
		this.children.push(node);
	}
}
