import { Validator } from "../validator";
import { RuntimeElementAttribute } from '../../../../runtime/element_attribute';
import { AstLayoutAttribute } from '../../../parser/parse/ast/layout/attribute';
import { RuntimeElementAttributeValue } from '../../../../runtime/element_attribute_value';

export function validateLayoutElementAttributeReservedValue(validator: Validator, attribute: AstLayoutAttribute, runtimeElementAttribute: RuntimeElementAttribute): string | undefined {
    const error = "Attribute value '" + attribute.value.getString() + "' is not a valid value for attribute '" + attribute.enduser_name + "' in element '" + attribute.element_enduser_name + "'";
    if (runtimeElementAttribute.reservedValues.length > 0) {
        const found = runtimeElementAttribute.reservedValues.find((value: RuntimeElementAttributeValue) => value.getText(validator.ast.language.id)?.includes(attribute.enduser_name));
        if (!found) {
            return error;
        }
        // Update the generate value of the attribute value
        attribute.generate_value = found.generate_name;
        return undefined;
    }
    return error;
};
