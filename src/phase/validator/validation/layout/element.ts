import { Validator } from "../validator";
import { validateLayoutBlock } from './block';
import { AstBlock } from "../../../parser/parse/ast/block";
import { RuntimeElement } from './../../../../runtime/element';
import { validateLayoutElementAttributes } from './element_attributes';
import { AstLayoutElement } from "./../../../parser/parse/ast/layout/element";
import { validatorMessageRenderer } from './../../../../common/message/message';
import { ValidatorMessageKeys } from "../../../../common/message/validator/validator";

export function validateLayoutElement(validator: Validator, parent_block: AstBlock, parent_element: AstLayoutElement | undefined, element: AstLayoutElement, runtime_element: RuntimeElement | undefined = undefined): void {
    // Try to get runtime element if not provided
    if (runtime_element === undefined) {
        // First check if element is a valid element
        runtime_element = Validator.getElementRuntime(validator.getLanguageId(), parent_element, element.enduser_name);

        // Second check if element is a valid style element
        if (runtime_element === undefined) {
            runtime_element = Validator.getStyleElementRuntime(validator.getLanguageId(), parent_element, element.enduser_name);
        }

        // Check if element is a valid element
        if (runtime_element === undefined) {
            validator.pushError(validatorMessageRenderer(validator.getLanguageId(), ValidatorMessageKeys.VALIDATOR_ELEMENT_NOT_VALID, element.enduser_name));
            return;
        }
    }

    element.generate_name = runtime_element.generate_name;
    element.generate_type = runtime_element.constructor.name;

    // Check attributes and styles
    validateLayoutElementAttributes(validator, element.enduser_name, runtime_element, element);

    // Check block
    validateLayoutBlock(validator, parent_block, parent_element, element, element.block);
};
