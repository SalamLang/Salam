import { Validator } from "../validator";
import { validateLayoutElement } from './element';
import { RuntimeElement } from './../../../../runtime/element';
import { validateLayoutElementStyleState } from './element_style_state';
import { AstLayoutElement } from "./../../../parser/parse/ast/layout/element";
import { validatorMessageRenderer } from './../../../../common/message/message';
import { RuntimeElementStyleState } from "../../../../runtime/element_style_state";
import { ValidatorMessageKeys } from "../../../../common/message/validator/validator";
import { AstBlock } from "../../../parser/parse/ast/block";

export function validateLayoutElementOrState(validator: Validator, parent_block: AstBlock, parent_element: AstLayoutElement | undefined, element: AstLayoutElement): void {
    const runtime_element: RuntimeElement | undefined = Validator.getElementRuntime(validator.getLanguageId(), parent_element, element.enduser_name);
    if (runtime_element === undefined) {
        console.log(element.enduser_name);
        // console.log(parent_element);
        console.log(parent_element?.enduser_name);
        console.log(runtime_element);
    }

    // First check if element is a valid element
    if (runtime_element !== undefined) {
        validateLayoutElement(validator, parent_block, parent_element, element, runtime_element);
        return;
    }
    
    // Second check if element is a valid style element
    const runtime_style_element: RuntimeElement | undefined = Validator.getStyleElementRuntime(validator.getLanguageId(), parent_element, element.enduser_name);
    if (runtime_style_element !== undefined) {
        validateLayoutElement(validator, parent_block, parent_element, element, runtime_style_element);
        return;
    }

    // Third check if element is a valid style state
    const runtimeStyleState: RuntimeElementStyleState | undefined = Validator.getElementStyleStateRuntime(validator.getLanguageId(), parent_element, element.enduser_name);
    if (runtimeStyleState !== undefined) {
        validateLayoutElementStyleState(validator, parent_element, element, runtimeStyleState);
    } else {
        validator.pushError(validatorMessageRenderer(validator.getLanguageId(), ValidatorMessageKeys.VALIDATOR_ELEMENT_OR_STYLE_STATE_NOT_VALID, element.enduser_name));
    }
};
