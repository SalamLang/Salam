import { Validation } from "./../validation";
import { RuntimeElement } from './../../../../runtime/element';
import { AstLayoutElement } from "./../../../parser/parse/ast/layout/element";
import { validateLayoutElementAttribute } from './element_attribute';

export function validateLayoutElement(validator: Validation, node: AstLayoutElement): void {
    const runtimeElement: RuntimeElement | undefined = validator.getElementRuntime(node.enduser_name);
    
    // Check if element is a valid element
    if (runtimeElement === undefined) {
        validator.pushError("Element '" + node.enduser_name + "' is not a valid element");
        return;
    }

    // Check attributes
    for (const attribute of node.attributes.items) {
        validateLayoutElementAttribute(validator, runtimeElement, attribute);
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
};
