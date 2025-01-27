import { Validator } from "../validator";
import { RuntimeElement } from '../../../../runtime/element';
import { AstLayoutElement } from "../../../parser/parse/ast/layout/element";
import { validateLayoutElementAttributeValue } from './element_attribute_value';
import { RuntimeElementAttribute } from '../../../../runtime/element_attribute';
import { AstLayoutAttribute } from "../../../parser/parse/ast/layout/attribute";
import { validatorMessages } from './../../../../common/message/validator/validator';
import { messageRenderer, ValidatorMessageKeys } from './../../../../common/message/message';
import { validateLayoutElementAttributeReservedValue } from './element_attribute_reserved_value';

export function validateLayoutElementAttribute(validator: Validator, runtimeElement: RuntimeElement, attribute: AstLayoutAttribute, element: AstLayoutElement): void {
    // TODO: .getText() is an array, so we should join it to get the string
    const element_name = runtimeElement.getText(validator.ast.language.id)?.join(" ") || "Unknown Element";
    const attribute_name = attribute.enduser_name;

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
        validator.pushError(messageRenderer(validatorMessages[validator.ast.language.id][ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_NOT_VALID], attribute_name, element_name));
        return;
    }

    // Update the generate name of the attribute value
    attribute.generate_name = runtimeElementAttribute.generate_name;
    attribute.generate_type = runtimeElementAttribute.constructor.name;

    // Handling content attribute (To save on element object)
    if (attribute.generate_type === "RuntimeGlobalAttributeContent") {
        element.content = attribute.value.getString().trim();
        if (element.content === "") {
            element.content = undefined;
        }
    } else {
        // Check if attributes values are valid for attribute with reserved values
        const error_reserved_value: string | undefined = validateLayoutElementAttributeReservedValue(validator, attribute, runtimeElementAttribute);
        if (error_reserved_value !== undefined) {
            // Check validation for attribute values
            const error_validate: string | undefined = validateLayoutElementAttributeValue(validator, attribute, runtimeElementAttribute);
            if (error_validate !== undefined) {
                validator.pushError(error_validate);
            }
        }
    }

    // Handling repeat attribute (To save on element object)
    if (attribute.generate_type === "RuntimeGlobalAttributeRepeat") {
        element.repeat = parseInt(attribute.value.getString());
    }
};
