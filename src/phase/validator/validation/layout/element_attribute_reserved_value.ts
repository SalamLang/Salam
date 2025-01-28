import { Validator } from "./../validator";
import { validatorMessageRenderer } from './../../../../common/message/message';
import { RuntimeElementAttribute } from './../../../../runtime/element_attribute';
import { AstLayoutAttribute } from './../../../parser/parse/ast/layout/attribute';
import { ValidatorMessageKeys } from './../../../../common/message/validator/validator';
import { RuntimeElementAttributeValue } from './../../../../runtime/element_attribute_value';

export function validateLayoutElementAttributeReservedValue(validator: Validator, attribute: AstLayoutAttribute, runtimeElementAttribute: RuntimeElementAttribute): string | undefined {
    const error: string = validatorMessageRenderer(validator.ast.language.id, ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_RESERVED_VALUE_NOT_VALID, attribute.value.getString(), attribute.enduser_name, attribute.element_enduser_name);
    
    if (runtimeElementAttribute.reservedValues.length > 0) {
        const found: RuntimeElementAttributeValue | undefined = runtimeElementAttribute.reservedValues.find((value: RuntimeElementAttributeValue) => value.getText(validator.ast.language.id)?.includes(attribute.value.getString()));
        if (! found) {
            return error;
        }
        // Update the generate value of the attribute value
        attribute.generate_value = found.generate_name;
        // Update the generate type of the attribute value
        attribute.generate_type = found.constructor.name;
        return undefined;
    }
    return error;
};
