import { Validator } from "../validator";
import { RuntimeElement } from '../../../../runtime/element';
import { validateLayoutElementAttributeValue } from './element_attribute_value';
import { RuntimeElementAttribute } from '../../../../runtime/element_attribute';
import { AstLayoutAttribute } from "../../../parser/parse/ast/layout/attribute";
import { RuntimeElementAttributeValue } from '../../../../runtime/element_attribute_value';

export function validateLayoutElementAttributeReservedValue(validator: Validator, node: AstLayoutAttribute, runtimeElementAttribute: RuntimeElementAttribute): string | undefined {
    const error = "Attribute value '" + node.value.getString() + "' is not a valid value for attribute '" + node.enduser_name + "' in element '" + node.element_enduser_name + "'";
    if (runtimeElementAttribute.reservedValues.length > 0) {
        const found = runtimeElementAttribute.reservedValues.find((value: RuntimeElementAttributeValue) => value.getText(validator.ast.language.id)?.includes(node.enduser_name));
        if (!found) {
            return error;
        }
        // Update the generate name of the attribute value
        node.generate_name = found.generate_name;
        return undefined;
    }
    return error;
};

export function validateLayoutElementAttribute(validator: Validator, runtimeElement: RuntimeElement, node: AstLayoutAttribute): void {
    const runtimeElementAttribute: RuntimeElementAttribute | undefined = validator.getElementAttributeRuntime(runtimeElement, node.enduser_name);

    // Check if attributes are valid attributes for the element
    if (runtimeElementAttribute === undefined) {
        validator.pushError("Attribute '" + node.enduser_name + "' is not a valid attribute for element '" + runtimeElement.getText(validator.ast.language.id) + "'");
        return;
    }

    // Check if attributes values are valid for attribute with reserved values
    const error_reserved_value: string | undefined = validateLayoutElementAttributeReservedValue(validator, node, runtimeElementAttribute);
    if (error_reserved_value === undefined) {
        return;
    } else {
        // Check validation for attribute values
        const error_validate: string | undefined = validateLayoutElementAttributeValue(validator, node, runtimeElementAttribute);
        if (error_validate !== undefined) {
            validator.pushError(error_validate);
        }
    }
};
