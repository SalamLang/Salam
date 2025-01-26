import { Validation } from "./../validation";
import { RuntimeElement } from './../../../../runtime/element';
import { RuntimeElementAttribute } from './../../../../runtime/element_attribute';
import { AstLayoutAttribute } from "./../../../parser/parse/ast/layout/attribute";
import { RuntimeElementAttributeValue } from './../../../../runtime/element_attribute_value';
import { RuntimeElementAttributeType } from './../../../../runtime/element_attribute_type';

export function validateLayoutElementAttribute(validator: Validation, runtimeElement: RuntimeElement, node: AstLayoutAttribute): void {
    const runtimeElementAttribute: RuntimeElementAttribute | undefined = validator.getElementAttributeRuntime(runtimeElement, node.enduser_name);

    // Check if attributes are valid attributes for the element
    if (runtimeElementAttribute === undefined) {
        validator.pushError("Attribute '" + node.enduser_name + "' is not a valid attribute for element '" + runtimeElement.getText(validator.ast.language.id) + "'");
        return;
    }

    // Check validation for attribute values
    if (validateLayoutElementAttributeValue(validator, node, runtimeElementAttribute) === false) {
        return;
    }
};
