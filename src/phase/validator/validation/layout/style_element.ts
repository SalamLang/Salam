import { Validator } from "../validator";
import { validateLayoutBlock } from './block';
import { AstBlock } from "../../../parser/parse/ast/block";
import { RuntimeStyleElement } from "../../../../runtime/style_element";
import { AstLayoutElement } from "./../../../parser/parse/ast/layout/element";
import { validatorMessageRenderer } from './../../../../common/message/message';
import { validateLayoutStyleElementAttributes } from "./style_element_attributes";
import { ValidatorMessageKeys } from "../../../../common/message/validator/validator";

export function validateLayoutStyleElement(validator: Validator, parent_block: AstBlock, parent_element: AstLayoutElement | undefined, element: AstLayoutElement, runtime_element: RuntimeStyleElement | undefined = undefined): void {
    // Try to get runtime element if not provided
    if (runtime_element === undefined) {
        // First check if element is a valid element
        runtime_element = Validator.getStyleElementRuntime(validator.getLanguageId(), parent_element, element.enduser_name);

        // Check if element is a valid element
        if (runtime_element === undefined) {
            validator.pushError(validatorMessageRenderer(validator.getLanguageId(), ValidatorMessageKeys.VALIDATOR_ELEMENT_NOT_VALID, element.enduser_name));
            return;
        }
    }

    element.generate_name = runtime_element.generate_name;
    element.generate_type = runtime_element.constructor.name;

    // Check if this element has belons to the parent
    // console.log("validateLayoutStyleElement:", element.kind, element.generate_name, runtime_element.getNotBelongsToItself(), parent_element);
    if (runtime_element && parent_element && runtime_element.not_belongs_to_itself === true && parent_element.enduser_name === element.enduser_name) {
        validator.pushError(`Cannot use element "${element.enduser_name}" in itself`);
    }

    // Check attributes and styles
    validateLayoutStyleElementAttributes(validator, element.enduser_name, runtime_element, element, parent_element);

    // Check block
    validateLayoutBlock(validator, parent_block, parent_element, element, element.block);
};
