import { Validator } from "../validator";
import { RuntimeElement } from '../../../../runtime/element';
import { validateLayoutElementAttributeValue } from './element_attribute_value';
import { RuntimeElementAttribute } from '../../../../runtime/element_attribute';
import { AstLayoutAttribute } from "../../../parser/parse/ast/layout/attribute";
import { validateLayoutElementAttributeReservedValue } from './element_attribute_reserved_value';

export function validateLayoutElementAttribute(validator: Validator, runtimeElement: RuntimeElement, node: AstLayoutAttribute): void {
    const element_name = runtimeElement.getText(validator.ast.language.id);
    const attribute_name = node.enduser_name;

    const validationChecks = [
        () => validator.getElementAttributeRuntime(runtimeElement, attribute_name),
        () => validator.getElementStyleAttributeRuntime(attribute_name),
        () => runtimeElement.is_mother
            ? validator.getElementGlobalMotherAttributeRuntime(attribute_name)
            : validator.getElementGlobalSingleAttributeRuntime(attribute_name),
        () => validator.getElementGlobalAttributeRuntime(attribute_name),
    ];

    // Try each check in order, stopping when a valid result is found
    const runtimeElementAttribute: RuntimeElementAttribute | undefined = validationChecks.reduce((result, check) => {
        return result || check();
    }, undefined as RuntimeElementAttribute | undefined);

    if (runtimeElementAttribute === undefined) {
        validator.pushError(
            `Attribute '${attribute_name}' is not a valid attribute for element '${element_name}'`
        );
        return;
    }

    // Update the generate name of the attribute value
    node.generate_name = runtimeElementAttribute.generate_name;
    node.generate_type = runtimeElementAttribute.constructor.name;

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
