import { Validation } from "./../validation";
import { RuntimeElement } from './../../../../runtime/element';
import { RuntimeElementAttribute } from './../../../../runtime/element_attribute';
import { AstLayoutAttribute } from "./../../../parser/parse/ast/layout/attribute";
import { RuntimeElementAttributeValue } from './../../../../runtime/element_attribute_value';

export function validateLayoutElementAttribute(validator: Validation, runtimeElement: RuntimeElement, node: AstLayoutAttribute): void {
    const runtimeElementAttribute: RuntimeElementAttribute | undefined = validator.getElementAttributeRuntime(runtimeElement, node.enduser_name);

    // Check if attributes are valid attributes for the element
    if (runtimeElementAttribute === undefined) {
        validator.pushError("Attribute '" + node.enduser_name + "' is not a valid attribute for element '" + runtimeElement.getText(validator.ast.language.id) + "'");
        return;
    }

    // TODO: Check validation for attribute values

    // Check if attributes values are valid for attribute with reserved values
    if (runtimeElementAttribute.reservedValues.length > 0) {
        const found = runtimeElementAttribute.reservedValues.find((value: RuntimeElementAttributeValue) => value.getText(validator.ast.language.id)?.includes(node.enduser_name));
        if (!found) {
            validator.pushError("Attribute value '" + node.value + "' is not a valid value for attribute '" + runtimeElementAttribute.getText(validator.ast.language.id) + "'");
        }
    }
};
