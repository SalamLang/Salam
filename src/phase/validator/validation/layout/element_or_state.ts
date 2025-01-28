import { Validator } from "./../validator";
import { validateLayoutElement } from './element';
import { RuntimeElement } from './../../../../runtime/element';
import { validateLayoutElementStyleState } from './element_style_state';
import { AstLayoutElement } from "./../../../parser/parse/ast/layout/element";
import { validatorMessageRenderer } from './../../../../common/message/message';
import { RuntimeElementStyleState } from "../../../../runtime/element_style_state";
import { ValidatorMessageKeys } from "../../../../common/message/validator/validator";

export function validateLayoutElementOrState(validator: Validator, parent_element: AstLayoutElement | undefined, element: AstLayoutElement): void {
    const runtime_element: RuntimeElement | undefined = validator.getElementRuntime(parent_element, element.enduser_name);

    if (runtime_element !== undefined) {
        validateLayoutElement(validator, parent_element, element, runtime_element);
    } else {
        const runtimeStyleState: RuntimeElementStyleState | undefined = validator.getElementStyleStateRuntime(parent_element, element.enduser_name);
        if (runtimeStyleState !== undefined) {
            validateLayoutElementStyleState(validator, parent_element, element, runtimeStyleState);
        } else {
            validator.pushError(validatorMessageRenderer(validator.ast.language.id, ValidatorMessageKeys.VALIDATOR_ELEMENT_OR_STYLE_STATE_NOT_VALID, element.enduser_name));
        }
    }
};
