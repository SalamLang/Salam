import { Validation } from "../validation";
import { RuntimeElementAttributeType } from './../../../../runtime/element_attribute_type';
import { RuntimeElementAttribute } from './../../../../runtime/element_attribute';
import { RuntimeElementAttributeValue } from './../../../../runtime/element_attribute_value';
import { AstLayoutAttribute } from './../../../parser/parse/ast/layout/attribute';

export function isUrl(value: string) {
    try {
        new URL(value);
        return true;
    } catch (e) {
        return false;
    }
}

// Check if attributes values are valid for attribute with reserved values
export function validateLayoutElementAttributeReservedValue(validator: Validation, node: AstLayoutAttribute, runtimeElementAttribute: RuntimeElementAttribute) {
    if (runtimeElementAttribute.reservedValues.length > 0) {
        const found = runtimeElementAttribute.reservedValues.find((value: RuntimeElementAttributeValue) => value.getText(validator.ast.language.id)?.includes(node.enduser_name));
        if (!found) {
            validator.pushError("Attribute value '" + node.value + "' is not a valid value for attribute '" + runtimeElementAttribute.getText(validator.ast.language.id) + "'");
        }
    }
}
export function validateLayoutElementAttributeValue(validator: Validation, node: AstLayoutAttribute, runtimeElementAttribute: RuntimeElementAttribute): string | undefined {
    const attr_name = runtimeElementAttribute.getText(validator.ast.language.id);
    const error = "Attribute value '" + node.value + "' is not a valid "+ runtimeElementAttribute.type +" for attribute '" + attr_name + "'";

    if (runtimeElementAttribute.type !== RuntimeElementAttributeType.String && node.value.getString() === "") {
        return "Attribute value '" + node.value + "' is not a valid "+ runtimeElementAttribute.type +" for attribute '" + attr_name + "' as it should not be empty";
    }

    switch (runtimeElementAttribute.type) {
        case RuntimeElementAttributeType.Url:
            if (!isUrl(node.value.getString())) {
                return error;
            }
            break;

        case RuntimeElementAttributeType.Urls:
            return undefined;

        case RuntimeElementAttributeType.String:
            return undefined;

        case RuntimeElementAttributeType.Strings:
            return undefined;

        case RuntimeElementAttributeType.AnyNoEmpty:
            return undefined;

        case RuntimeElementAttributeType.Sizes:
            return undefined;

        case RuntimeElementAttributeType.Size:
            return undefined;

        case RuntimeElementAttributeType.PositiveInt:
            return undefined;

        case RuntimeElementAttributeType.IDs:
            return undefined;

        case RuntimeElementAttributeType.ID:
            // TODO: make sure ID name defined somewhere
            return undefined;

        case RuntimeElementAttributeType.CSSMedia:
            // TODO: make sure it is a valid CSS media
            return undefined;

        case RuntimeElementAttributeType.Coords:
            const coords = node.value.getString().split(",");
            if (coords.length !== 2) {
                return error;
            } else if (isNaN(Number(coords[0])) || isNaN(Number(coords[1]))) {
                return error;
            }
            return undefined;

        case RuntimeElementAttributeType.Color:
            if (node.value.getString().startsWith("#") === true) {
                if (node.value.getString().length !== 7 || node.value.getString().length !== 4 || !/^#[0-9A-F]{6}$/i.test(node.value.getString())) {
                    return error;
                }
            }
            return undefined;

        case RuntimeElementAttributeType.Colors:
            // TODO: make sure it is array of valid colors
            return undefined;

        case RuntimeElementAttributeType.Number:
            if (isNaN(Number(node.value.getString()))) {
                return error;
            }
            return undefined;

        case RuntimeElementAttributeType.Boolean:
            if (node.value.getString() !== "true" && node.value.getString() !== "false") {
                return error;
            }
            return undefined;

        case RuntimeElementAttributeType.Date: {
            // Example: 2021-01-01
            const datePattern = /^\d{4}-\d{2}-\d{2}$/;
            const value = node.value.getString();
            if (!datePattern.test(value) || isNaN(Date.parse(value))) {
                return error;
            }
            return undefined;
        }
    
        case RuntimeElementAttributeType.Time: {
            // Example: 12:00
            const timePattern = /^([01]?[0-9]|2[0-3]):([0-5][0-9])$/;
            const value = node.value.getString();
            if (!timePattern.test(value) || isNaN(Date.parse(`1970-01-01T${value}:00Z`))) {
                return error;
            }
            return undefined;
        }
    
        case RuntimeElementAttributeType.Times: {
            // TODO: make sure it is array of valid times
        }
    
        case RuntimeElementAttributeType.DateTime: {
            // Example: 2021-01-01T12:00:00
            const dateTimePattern = /^\d{4}-\d{2}-\d{2}T([01]?[0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9])$/;
            const value = node.value.getString();
            if (!dateTimePattern.test(value) || isNaN(Date.parse(value))) {
                return error;
            }
            return undefined;
        }
      
        case RuntimeElementAttributeType.NumberOr2Numbers: {
            const parts = node.value.getString().split(" ");
            if (parts.length > 2) {
                return error;
            }
            for (const part of parts) {
                if (isNaN(Number(part))) {
                    return error;
                }
            }
            return undefined;
        }
    }
};
