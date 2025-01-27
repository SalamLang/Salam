import { Validator } from "../validator";
import { RuntimeElement } from '../../../../runtime/element';
import { validateLayoutElementAttribute } from './element_attribute';
import { AstLayoutElement } from "../../../parser/parse/ast/layout/element";
import { AstLayoutAttributes } from "../../../parser/parse/ast/layout/attributes";
import { validatorMessages } from './../../../../common/message/validator/validator';
import { messageRenderer, ValidatorMessageKeys } from './../../../../common/message/message';

export function validateLayoutElementAttributes(validator: Validator, element_enduser_name: string, runtimeElement: RuntimeElement, attributes: AstLayoutAttributes, element: AstLayoutElement): void {
    for (const attribute of attributes.items) {
        validateLayoutElementAttribute(validator, runtimeElement, attribute, element);
    }

    // Check if required attributes are not present
    for (const runtimeAttribute of runtimeElement.attributes) {
        if (runtimeAttribute.is_required) {
            const found = attributes.items.find((nodeAttribute) => runtimeAttribute.getText(validator.ast.language.id)?.includes(nodeAttribute.enduser_name));
            if (!found) {
                validator.pushError(messageRenderer(validatorMessages[validator.ast.language.id][ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_REQUIRED], runtimeAttribute.getText(validator.ast.language.id), element_enduser_name));
            }
        }
    }
};
