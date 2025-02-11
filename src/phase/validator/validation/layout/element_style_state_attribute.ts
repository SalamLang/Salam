import { Validator } from "../validator";
import { RuntimeElement } from './../../../../runtime/element';
import { AstLayoutElement } from "./../../../parser/parse/ast/layout/element";
import { validateLayoutElementAttributeValue } from './element_attribute_value';
import { validatorMessageRenderer } from './../../../../common/message/message';
import { RuntimeElementAttribute } from './../../../../runtime/element_attribute';
import { AstLayoutAttribute } from "./../../../parser/parse/ast/layout/attribute";
import { RuntimeElementStyleState } from './../../../../runtime/element_style_state';
import { ValidatorMessageKeys } from './../../../../common/message/validator/validator';
import { validateLayoutElementAttributeReservedValue } from './element_attribute_reserved_value';

export function validateLayoutElementStyleStateAttribute(validator: Validator, runtime_element_style_state: RuntimeElementStyleState, attribute: AstLayoutAttribute, element: AstLayoutElement, main_element: AstLayoutElement | undefined): void {
    const element_name: string = element.enduser_name;
    const attribute_name: string = attribute.enduser_name;

    const runtimeElementAttribute: RuntimeElementAttribute | undefined = Validator.getElementAttributeStyle(validator.getLanguageId(), attribute_name);
    if (runtimeElementAttribute === undefined) {
        validator.pushError(validatorMessageRenderer(validator.getLanguageId(), ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_NOT_VALID, attribute_name, element_name));
        return;
    }

    // Update the kind, generate name, generate type of the attribute value
    attribute.kind = runtimeElementAttribute.kind;
    attribute.generate_name = runtimeElementAttribute.generate_name;
    attribute.generate_type = runtimeElementAttribute.constructor.name;

    // Check if attributes values are valid for attribute with reserved values
    const error_reserved_value: string | undefined = validateLayoutElementAttributeReservedValue(validator, attribute, runtimeElementAttribute);
    if (error_reserved_value !== undefined) {
        // Check validation for attribute values
        const error_validate: string | undefined = validateLayoutElementAttributeValue(validator, attribute, runtimeElementAttribute);
        if (error_validate !== undefined) {
            validator.pushError(error_validate);
        }
    }
};
