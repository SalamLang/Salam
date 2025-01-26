import { Validator } from "../validator";
import { validateLayoutBlock } from './block';
import { RuntimeElement } from '../../../../runtime/element';
import { validateLayoutElementAttributes } from './element_attributes';
import { AstLayoutElement } from "../../../parser/parse/ast/layout/element";

export function validateLayoutElement(validator: Validator, node: AstLayoutElement): void {
    const runtimeElement: RuntimeElement | undefined = validator.getElementRuntime(node.enduser_name);

    // Check if element is a valid element
    if (runtimeElement === undefined) {
        validator.pushError("Element '" + node.enduser_name + "' is not a valid element");
        return;
    }

    node.generate_name = runtimeElement.generate_name;
    node.generate_type = runtimeElement.constructor.name;

    // Check attributes
    validateLayoutElementAttributes(validator, node.enduser_name, runtimeElement, node.attributes, node);

    // Check global attributes
    validateLayoutElementAttributes(validator, node.enduser_name, runtimeElement, node.globalAttributes, node);

    // Check block
    validateLayoutBlock(validator, node.block);
};
