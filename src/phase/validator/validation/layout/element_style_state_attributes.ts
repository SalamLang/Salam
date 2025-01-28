import { Validator } from "../validator";
// import { validateLayoutElementAttribute } from './element_attribute';
import { AstLayoutElement } from "../../../parser/parse/ast/layout/element";
// import { validatorMessageRenderer } from '../../../../common/message/message';
// import { AstLayoutAttribute } from "../../../parser/parse/ast/layout/attribute";
import { RuntimeElementStyleState } from '../../../../runtime/element_style_state';
// import { ValidatorMessageKeys } from '../../../../common/message/validator/validator';
import { validateLayoutElementStyleStateAttribute } from './element_style_state_attribute';

export function validateLayoutElementStyleStateAttributes(validator: Validator, element_enduser_name: string, runtime_element_style_state: RuntimeElementStyleState, element: AstLayoutElement): void {
    for (const attribute of element.styles.items) {
        validateLayoutElementStyleStateAttribute(validator, runtime_element_style_state, attribute, element);
    }

    // Check if user has provided any extra attributes which are not supported
    // for (const attribute of element.styles.items) {
    //     if (attribute.generate_name !== undefined && ! Validator.hasElementAllAttributeRuntime(validator.getLanguageId(), runtime_element_style_state, attribute.enduser_name)) {
    //         validator.pushError(validatorMessageRenderer(validator.getLanguageId(), ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_NOT_SUPPORTED, attribute.enduser_name, element_enduser_name));
    //     }
    // }
};
