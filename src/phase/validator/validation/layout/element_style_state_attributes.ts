import { Validator } from "../validator";
import { validateLayoutElementAttribute } from './element_attribute';
import { AstLayoutElement } from "../../../parser/parse/ast/layout/element";
import { validatorMessageRenderer } from '../../../../common/message/message';
import { AstLayoutAttribute } from "../../../parser/parse/ast/layout/attribute";
import { RuntimeElementStyleState } from '../../../../runtime/element_style_state';
import { ValidatorMessageKeys } from '../../../../common/message/validator/validator';

export function validateLayoutElementStyleStateAttributes(validator: Validator, element_enduser_name: string, element_style_state: RuntimeElementStyleState, element: AstLayoutElement): void {
    for (const attribute of element.attributes.items) {
        validateLayoutElementAttribute(validator, element_style_state, attribute, element);
    }

    // Check if user has provided any extra attributes which are not supported
    for (const attribute of element.attributes.items) {
        if (attribute.generate_name !== undefined && ! validator.hasElementAllAttributeRuntime(element_style_state, attribute.enduser_name)) {
            validator.pushError(validatorMessageRenderer(validator.ast.language.id, ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_NOT_SUPPORTED, attribute.enduser_name, element_enduser_name));
        }
    }
};
