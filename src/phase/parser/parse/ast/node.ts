import { stringify } from './../../../../serializer';

export class AstNode {
	type: string;

	constructor(type: string) {
		this.type = type;
	}

	stringify(wantsJson: boolean = true): string | object {
		const obj: object = {
			type: this.type
		};
		return stringify(obj, wantsJson);
	}
}
