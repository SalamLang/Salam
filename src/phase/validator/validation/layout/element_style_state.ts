import { Validator } from "../validator";
import { validateLayoutBlock } from "./block";
import { AstBlock } from "../../../parser/parse/ast/block";
import { AstLayoutElement } from "./../../../parser/parse/ast/layout/element";
import { validatorMessageRenderer } from './../../../../common/message/message';
import { RuntimeElementStyleState } from "../../../../runtime/element_style_state";
import { ValidatorMessageKeys } from "../../../../common/message/validator/validator";
import { AstLayoutElementKind } from './../../../parser/parse/ast/layout/element_kind';
import { validateLayoutElementStyleStateAttributes } from './element_style_state_attributes';

export function validateLayoutElementStyleState(validator: Validator, parent_element: AstLayoutElement | undefined, element: AstLayoutElement, element_style_state: RuntimeElementStyleState | undefined = undefined, parent_block: AstBlock): void {
    // Try to get runtime element if not provided
    if (element_style_state === undefined) {
        element_style_state = Validator.getElementStyleStateRuntime(validator.getLanguageId(), parent_element, element.enduser_name);

        // Check if element is a valid element
        if (element_style_state === undefined) {
            validator.pushError(validatorMessageRenderer(validator.getLanguageId(), ValidatorMessageKeys.VALIDATOR_ELEMENT_STYLE_STATE_NOT_VALID, element.enduser_name));
            return;
        }
    }

    element.kind = AstLayoutElementKind.StyleState;
    element.generate_name = element_style_state.generate_name;
    element.generate_type = element_style_state.constructor.name;
    element.built_in_selector = parent_element?.built_in_selector;

    if (parent_element && element.kind === "StyleState" && parent_element.kind === "StyleState") {
        validator.pushError(`Cannot have '${element.enduser_name}' style state inner of '${parent_element.enduser_name}' style state.`);
    }

    // Check attributes and global attributes
    validateLayoutElementStyleStateAttributes(validator, element.enduser_name, element_style_state, element, parent_element);

    // Check block
    validateLayoutBlock(validator, parent_block, parent_element, element, element.block);
};
