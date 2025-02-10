import { Validator } from "../validator";
import { RuntimeElement } from './../../../../runtime/element';
import { AstLayoutElement } from "./../../../parser/parse/ast/layout/element";
import { validateLayoutElementAttributeValue } from './element_attribute_value';
import { validatorMessageRenderer } from './../../../../common/message/message';
import { RuntimeElementAttribute } from './../../../../runtime/element_attribute';
import { AstLayoutAttribute } from "./../../../parser/parse/ast/layout/attribute";
import { ValidatorMessageKeys } from './../../../../common/message/validator/validator';
import { validateLayoutElementAttributeReservedValue } from './element_attribute_reserved_value';

export function validateLayoutElementStyleAttribute(validator: Validator, runtimeElement: RuntimeElement, attribute: AstLayoutAttribute, element: AstLayoutElement): void {
    const element_name: string = element.enduser_name;
    const attribute_name: string = attribute.enduser_name;

    const runtimeElementStyleAttribute: RuntimeElementAttribute | undefined = Validator.getElementAttributeStyle(validator.getLanguageId(), attribute_name);
    if (runtimeElementStyleAttribute === undefined) {
        validator.pushError(validatorMessageRenderer(validator.getLanguageId(), ValidatorMessageKeys.VALIDATOR_STYLE_ATTRIBUTE_NOT_VALID, attribute_name, element_name));
        return;
    }

    // Update the kind, generate name, generate type of the attribute value
    attribute.kind = runtimeElementStyleAttribute.kind;
    attribute.generate_name = runtimeElementStyleAttribute.generate_name;
    attribute.generate_type = runtimeElementStyleAttribute.constructor.name;

    // Check if attributes values are valid for attribute with reserved values
    const error_reserved_value: string | undefined = validateLayoutElementAttributeReservedValue(validator, attribute, runtimeElementStyleAttribute);
    if (error_reserved_value !== undefined) {
        // Check validation for attribute values
        const error_validate: string | undefined = validateLayoutElementAttributeValue(validator, attribute, runtimeElementStyleAttribute);
        if (error_validate !== undefined) {
            validator.pushError(error_validate);
        }
    }
};
