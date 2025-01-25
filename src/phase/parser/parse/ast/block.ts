import { AstNode } from './node';
import { stringify } from '../../../../serializer';

export class AstBlock extends AstNode {
	children: AstNode[] = [];

	constructor() {
		super("Block");
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
