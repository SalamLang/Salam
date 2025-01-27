import { Validator } from "./../validator";
import { validateLayoutBlock } from './block';
import { RuntimeElement } from './../../../../runtime/element';
import { validateLayoutElementAttributes } from './element_attributes';
import { AstLayoutElement } from "./../../../parser/parse/ast/layout/element";
import { validatorMessages } from './../../../../common/message/validator/validator';
import { messageRenderer, ValidatorMessageKeys } from './../../../../common/message/message';

export function validateLayoutElement(validator: Validator, parent_element: AstLayoutElement | undefined, element: AstLayoutElement): void {
    const runtimeElement: RuntimeElement | undefined = validator.getElementRuntime(parent_element, element.enduser_name);

    // Check if element is a valid element
    if (runtimeElement === undefined) {
        validator.pushError(messageRenderer(validatorMessages[validator.ast.language.id][ValidatorMessageKeys.VALIDATOR_ELEMENT_NOT_VALID], element.enduser_name));
        return;
    }

    element.generate_name = runtimeElement.generate_name;
    element.generate_type = runtimeElement.constructor.name;

    // Check attributes and global attributes
    validateLayoutElementAttributes(validator, element.enduser_name, runtimeElement, element);

    // Check block
    validateLayoutBlock(validator, parent_element, element, element.block);
};
