import { Validator } from "./../validator";
import { RuntimeElement } from './../../../../runtime/element';
import { validateLayoutElementAttribute } from './element_attribute';
import { AstLayoutElement } from "./../../../parser/parse/ast/layout/element";
import { AstLayoutAttribute } from "./../../../parser/parse/ast/layout/attribute";
import { AstLayoutAttributes } from "./../../../parser/parse/ast/layout/attributes";
import { validatorMessages } from './../../../../common/message/validator/validator';
import { messageRenderer, ValidatorMessageKeys } from './../../../../common/message/message';

export function validateLayoutElementAttributes(validator: Validator, element_enduser_name: string, runtimeElement: RuntimeElement, element: AstLayoutElement): void {
    for (const attribute of element.attributes.items) {
        validateLayoutElementAttribute(validator, runtimeElement, attribute, element);
    }
    for (const attribute of element.globalAttributes.items) {
        validateLayoutElementAttribute(validator, runtimeElement, attribute, element);
    }

    // Check if required attributes are not present
    for (const runtimeAttribute of runtimeElement.attributes) {
        if (runtimeAttribute.is_required && runtimeAttribute.generate_name !== undefined) {
            const foundAttributes: AstLayoutAttribute | undefined = element.attributes.getByGenerateName(runtimeAttribute.generate_name);
            const foundGlobalAttributes: AstLayoutAttribute | undefined = element.globalAttributes.getByGenerateName(runtimeAttribute.generate_name);
            
            if (foundAttributes === undefined && foundGlobalAttributes === undefined) {
                // TODO: .getText() is an array, so we should join it to get the string
                const attribute_name = runtimeAttribute.getText(validator.ast.language.id)?.join(" ") || "Unknown Attribute";
                validator.pushError(messageRenderer(validatorMessages[validator.ast.language.id][ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_REQUIRED], attribute_name, element_enduser_name));
            }
        }
    }
};
