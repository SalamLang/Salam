import { Validator } from "../validator";
import { validateLayoutElementAttribute } from './element_attribute';
import { RuntimeStyleElement } from "../../../../runtime/style_element";
import { AstLayoutElement } from "./../../../parser/parse/ast/layout/element";
import { validatorMessageRenderer } from './../../../../common/message/message';
import { AstLayoutAttribute } from "./../../../parser/parse/ast/layout/attribute";
import { ValidatorMessageKeys } from './../../../../common/message/validator/validator';

export function validateLayoutStyleElementAttributes(validator: Validator, element_enduser_name: string, runtime_element: RuntimeStyleElement, element: AstLayoutElement, parent_element: AstLayoutElement | undefined): void {
    // console.log(element.kind, element.enduser_name, element.type);
    // if (parent_element && element.generate_type?.startsWith("RuntimeStyle") && parent_element.generate_type?.startsWith("RuntimeStyle")) {
    //     validator.pushError("Cannot have this element inside that element.");
    //     validator.pushError(`Cannot have '${element.enduser_name}' style  inner of '${parent_element.enduser_name}' style state.`);
    // }

    // Check layout attributes
    for (const attribute of element.attributes.items) {
        validateLayoutElementAttribute(validator, runtime_element, attribute, element);
    }

    // Check if required attributes are not present
    for (const runtimeAttribute of runtime_element.attributes) {
        if (runtimeAttribute.is_required && runtimeAttribute.generate_name !== undefined) {
            const foundAttributes: AstLayoutAttribute | undefined = element.attributes.getByGenerateName(runtimeAttribute.generate_name);
            
            if (foundAttributes === undefined) {
                // TODO: .getText() is an array, so we should join it to get the string
                const attribute_name: string = runtimeAttribute.getText(validator.getLanguageId())?.join(" ") || "Unknown Attribute";
                validator.pushError(validatorMessageRenderer(validator.getLanguageId(), ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_REQUIRED, attribute_name, element_enduser_name));
            }
        }
    }

    // Check if user has provided any extra attributes which are not supported
    for (const attribute of element.attributes.items) {
        if (attribute.generate_name !== undefined && ! Validator.hasElementAllAttributeRuntime(validator.getLanguageId(), runtime_element, attribute.enduser_name)) {
            validator.pushError(validatorMessageRenderer(validator.getLanguageId(), ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_NOT_SUPPORTED, attribute.enduser_name, element_enduser_name));
        }
    }

    // Check styles
    if (element.styles.items.length > 0) {
        if (element.built_in_selector === undefined) {
            if (parent_element && parent_element.built_in_selector) {
                element.built_in_selector = parent_element.built_in_selector;
            } else {
                element.generateBuiltInSelector(validator);
            }
        }

        for (const attribute of element.styles.items) {
            // console.log("attr:", attribute.enduser_name);
            validateLayoutElementAttribute(validator, runtime_element, attribute, element);
        }
    }
};
