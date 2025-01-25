import { Validation } from "./../validation";
import { RuntimeElement } from './../../../../runtime/element';
import { AstLayoutElement } from "./../../../parser/parse/ast/layout/element";
import { RuntimeElementAttribute } from './../../../../runtime/element_attribute';

export function validateLayoutElementAttribute(validator: Validation, runtimeElement: RuntimeElement, node: AstLayoutElement) {
    const runtimeElementAttribute: RuntimeElementAttribute | undefined = validator.getElementAttributeRuntime(runtimeElement, node.enduser_name);

    if (runtimeElementAttribute === undefined) {
        validator.pushError("Attribute '" + node.enduser_name + "' is not a valid attribute for element '" + runtimeElement.getText(validator.ast.language.id) + "'");
        return;
    }

    // Check if attributes are valid attributes for the element
    if (runtimeElement?.attributes.length > 0 && node.attributes.length > 0) {
        for (const nodeAttribute of node.attributes.items) {
            const found = runtimeElement.hasAttribute(validator.ast.language.id, nodeAttribute.enduser_name);
            if (!found) {
                validator.pushError("Attribute '" + nodeAttribute.enduser_name + "' is not a valid attribute for element '" + node.enduser_name + "'");
            }
        }
    }

    // Check if required attributes are present
    for (const runtimeAttribute of runtimeElement.attributes) {
        if (runtimeAttribute.is_required) {
            const found = node.attributes.items.find((nodeAttribute) => runtimeAttribute.getText(validator.ast.language.id)?.includes(nodeAttribute.enduser_name));
            if (!found) {
                validator.pushError("Attribute '" + runtimeAttribute.getText(validator.ast.language.id) + "' is required for element '" + node.enduser_name + "' but not found");
            }
        }
    }

    // TODO: Check if attributes values are valid for attribute with reserved values
    // TODO: Check validation for attribute values
};
