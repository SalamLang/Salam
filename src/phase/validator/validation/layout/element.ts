import { Validator } from "../validator";
import { validateLayoutBlock } from './block';
import { RuntimeElement } from '../../../../runtime/element';
import { validateLayoutElementAttributes } from './element_attributes';
import { AstLayoutElement } from "../../../parser/parse/ast/layout/element";

export function validateLayoutElement(validator: Validator, parent_element: AstLayoutElement | undefined, element: AstLayoutElement): void {
    const runtimeElement: RuntimeElement | undefined = validator.getElementRuntime(parent_element, element.enduser_name);

    // Check if element is a valid element
    if (runtimeElement === undefined) {
        validator.pushError("Element '" + element.enduser_name + "' is not a valid element");
        return;
    }

    element.generate_name = runtimeElement.generate_name;
    element.generate_type = runtimeElement.constructor.name;

    // Check attributes
    validateLayoutElementAttributes(validator, element.enduser_name, runtimeElement, element.attributes, element);

    // Check global attributes
    validateLayoutElementAttributes(validator, element.enduser_name, runtimeElement, element.globalAttributes, element);

    // Check block
    validateLayoutBlock(validator, parent_element, element, element.block);
};
