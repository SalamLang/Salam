import { Validator } from "./../validator";
import { RuntimeElement } from './../../../../runtime/element';
import { validateLayoutElementAttribute } from './element_attribute';
import { AstLayoutElement } from "./../../../parser/parse/ast/layout/element";
import { validatorMessageRenderer } from './../../../../common/message/message';
import { AstLayoutAttribute } from "./../../../parser/parse/ast/layout/attribute";
import { ValidatorMessageKeys } from './../../../../common/message/validator/validator';

export function validateLayoutElementAttributes(validator: Validator, element_enduser_name: string, runtimeElement: RuntimeElement, element: AstLayoutElement): void {
    for (const attribute of element.attributes.items) {
        validateLayoutElementAttribute(validator, runtimeElement, attribute, element);
    }

    // Check if required attributes are not present
    for (const runtimeAttribute of runtimeElement.attributes) {
        if (runtimeAttribute.is_required && runtimeAttribute.generate_name !== undefined) {
            const foundAttributes: AstLayoutAttribute | undefined = element.attributes.getByGenerateName(runtimeAttribute.generate_name);
            
            if (foundAttributes === undefined) {
                // TODO: .getText() is an array, so we should join it to get the string
                const attribute_name = runtimeAttribute.getText(validator.ast.language.id)?.join(" ") || "Unknown Attribute";
                validator.pushError(validatorMessageRenderer(validator.ast.language.id, ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_REQUIRED, attribute_name, element_enduser_name));
            }
        }
    }

    // Check if user has provided any extra attributes which are not supported
    for (const attribute of element.attributes.items) {
        if (! runtimeElement.hasAttribute(validator.ast.language.id, attribute.enduser_name) && attribute.generate_name !== undefined) {
            validator.pushError(validatorMessageRenderer(validator.ast.language.id, ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_NOT_SUPPORTED, attribute.generate_name, element_enduser_name));
        }
    }
};
