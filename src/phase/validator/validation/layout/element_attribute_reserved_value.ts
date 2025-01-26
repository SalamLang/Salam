import { Validator } from "../validator";
import { RuntimeElementAttribute } from '../../../../runtime/element_attribute';
import { AstLayoutAttribute } from '../../../parser/parse/ast/layout/attribute';
import { RuntimeElementAttributeValue } from '../../../../runtime/element_attribute_value';

export function validateLayoutElementAttributeReservedValue(validator: Validator, node: AstLayoutAttribute, runtimeElementAttribute: RuntimeElementAttribute): string | undefined {
    const error = "Attribute value '" + node.value.getString() + "' is not a valid value for attribute '" + node.enduser_name + "' in element '" + node.element_enduser_name + "'";
    if (runtimeElementAttribute.reservedValues.length > 0) {
        const found = runtimeElementAttribute.reservedValues.find((value: RuntimeElementAttributeValue) => value.getText(validator.ast.language.id)?.includes(node.enduser_name));
        if (!found) {
            return error;
        }
        // Update the generate value of the attribute value
        node.generate_value = found.generate_name;
        return undefined;
    }
    return error;
};
