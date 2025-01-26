import { Validator } from "../validator";
import { RuntimeElement } from '../../../../runtime/element';
import { validateLayoutElementAttributeValue } from './element_attribute_value';
import { RuntimeElementAttribute } from '../../../../runtime/element_attribute';
import { AstLayoutAttribute } from "../../../parser/parse/ast/layout/attribute";
import { validateLayoutElementAttributeReservedValue } from './element_attribute_reserved_value';


export function validateLayoutElementAttribute(validator: Validator, runtimeElement: RuntimeElement, node: AstLayoutAttribute): void {
    const element_name = runtimeElement.getText(validator.ast.language.id);
    let runtimeElementAttribute: RuntimeElementAttribute | undefined = validator.getElementAttributeRuntime(runtimeElement, node.enduser_name);

    // Check if this attribute is not a valid attribute for this element
    if (runtimeElementAttribute === undefined) {
        // Check if this attribute is a valid style attribute for this element
        runtimeElementAttribute = validator.getElementStyleAttributeRuntime(node.enduser_name);

        if (runtimeElementAttribute === undefined) {
            validator.pushError("Attribute '" + node.enduser_name + "' is not a valid attribute for element '" + element_name + "'");
            return;
        }
    }

    // Update the generate name of the attribute value
    node.generate_name = runtimeElementAttribute.generate_name;

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
