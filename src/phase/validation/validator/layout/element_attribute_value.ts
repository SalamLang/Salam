import { Validation } from "../validation";
import { RuntimeElementAttribute } from './../../../../runtime/element_attribute';
import { AstLayoutAttribute } from './../../../parser/parse/ast/layout/attribute';
import { RuntimeElementAttributeType } from './../../../../runtime/element_attribute_type';

export function validateLayoutElementAttributeValue(validator: Validation, node: AstLayoutAttribute, runtimeElementAttribute: RuntimeElementAttribute): string | undefined {
    const attr_name = runtimeElementAttribute.getText(validator.ast.language.id);
    const value = node.value.getString();

    const error = "Attribute value '" + value + "' is not a valid "+ runtimeElementAttribute.type +" for attribute '" + attr_name + "' in element '" + node.element_enduser_name + "'";

    if (runtimeElementAttribute.type !== RuntimeElementAttributeType.String && value === "") {
        return "Attribute value '" + value + "' is not a valid "+ runtimeElementAttribute.type +" for attribute '" + attr_name + "' as it should not be empty in element '" + node.element_enduser_name + "'";
    }

    switch (runtimeElementAttribute.type) {
        case RuntimeElementAttributeType.Url: {
            try {
                new URL(value);
                return undefined;
            } catch (e) {
                return error;
            }
        }

        case RuntimeElementAttributeType.Urls: {
            return undefined;
        }

        case RuntimeElementAttributeType.String: {
            return undefined;
        }

        case RuntimeElementAttributeType.Strings: {
            return undefined;
        }

        case RuntimeElementAttributeType.StringNoEmpty: {
            return undefined;
        }

        case RuntimeElementAttributeType.Sizes: {
            return undefined;
        }

        case RuntimeElementAttributeType.Size: {
            return undefined;
        }

        case RuntimeElementAttributeType.PositiveInt: {
            return undefined;
        }

        case RuntimeElementAttributeType.IDs: {
            return undefined;
        }

        case RuntimeElementAttributeType.ID: {
            // TODO: make sure ID name defined somewhere
            return undefined;
        }

        case RuntimeElementAttributeType.CSSMedia: {
            // TODO: make sure it is a valid CSS media
            return undefined;
        }

        case RuntimeElementAttributeType.Coords: {
            const coords = value.split(",");
            if (coords.length !== 2) {
                return error;
            } else if (isNaN(Number(coords[0])) || isNaN(Number(coords[1]))) {
                return error;
            }
            return undefined;
        }

        case RuntimeElementAttributeType.Color: {
            if (value.startsWith("#") === true) {
                if ((value.length !== 7 && value.length !== 4) || !/^#[0-9A-F]{6}$/i.test(value)) {
                    return error;
                }
            }
            return undefined;
        }

        case RuntimeElementAttributeType.Colors: {
            // TODO: make sure it is array of valid colors
            return undefined;
        }

        case RuntimeElementAttributeType.Number: {
            if (isNaN(Number(value))) {
                return error;
            }
            return undefined;
        }

        case RuntimeElementAttributeType.Boolean: {
            if (value !== "true" && value !== "false") {
                return error;
            }
            return undefined;
        }

        case RuntimeElementAttributeType.Date: {
            // Example: 2021-01-01
            const datePattern = /^\d{4}-\d{2}-\d{2}$/;
            if (!datePattern.test(value) || isNaN(Date.parse(value))) {
                return error;
            }
            return undefined;
        }
    
        case RuntimeElementAttributeType.Time: {
            // Example: 12:00
            const timePattern = /^([01]?[0-9]|2[0-3]):([0-5][0-9])$/;
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
            if (!dateTimePattern.test(value) || isNaN(Date.parse(value))) {
                return error;
            }
            return undefined;
        }
      
        case RuntimeElementAttributeType.NumberOr2Numbers: {
            const parts = value.split(" ");
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
