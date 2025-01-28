import { Validator } from "./../validator";
import { AstLayoutElement } from "./../../../parser/parse/ast/layout/element";
import { validatorMessageRenderer } from './../../../../common/message/message';
import { RuntimeElementStyleState } from "../../../../runtime/element_style_state";
import { ValidatorMessageKeys } from "../../../../common/message/validator/validator";
import { AstLayoutElementKind } from './../../../parser/parse/ast/layout/element_kind';
import { validateLayoutElementStyleStateAttributes } from './element_style_state_attributes';

export function validateLayoutElementStyleState(validator: Validator, parent_element: AstLayoutElement | undefined, element: AstLayoutElement, element_style_state: RuntimeElementStyleState | undefined = undefined): void {
    // Try to get runtime element if not provided
    if (element_style_state === undefined) {
        element_style_state = validator.getElementStyleStateRuntime(parent_element, element.enduser_name);

        // Check if element is a valid element
        if (element_style_state === undefined) {
            validator.pushError(validatorMessageRenderer(validator.ast.language.id, ValidatorMessageKeys.VALIDATOR_ELEMENT_STYLE_STATE_NOT_VALID, element.enduser_name));
            return;
        }
    }

    element.generate_name = element_style_state.generate_name;
    element.kind = AstLayoutElementKind.StyleState;
    element.generate_type = element_style_state.constructor.name;
    
    // Check attributes and global attributes
    validateLayoutElementStyleStateAttributes(validator, element.enduser_name, element_style_state, element);

    // Check block
    // TODO
};
