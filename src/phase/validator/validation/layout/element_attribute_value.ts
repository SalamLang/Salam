import { isUrl } from './../is-url';
import { Validator } from "../validator";
import { validatorMessageRenderer } from './../../../../common/message/message';
import { RuntimeElementAttribute } from './../../../../runtime/element_attribute';
import { AstLayoutAttribute } from './../../../parser/parse/ast/layout/attribute';
import { ValidatorMessageKeys } from './../../../../common/message/validator/validator';
import { RuntimeElementAttributeType } from './../../../../runtime/element_attribute_type';
import { RuntimeElementAttributeOutputType } from './../../../../runtime/element_attribute_output_type';

export function validateLayoutElementAttributeValue(validator: Validator, attribute: AstLayoutAttribute, runtimeElementAttribute: RuntimeElementAttribute): string | undefined {
    // TOOD: .getText() is an array, so we should join it to get the string
    const attr_name: string = runtimeElementAttribute.getText(validator.getLanguageId())?.join(" ") || "Unknown Attribute";
    const value: string = attribute.value.getString();

    const error: string = validatorMessageRenderer(validator.getLanguageId(), ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_VALUE_NOT_VALID, value, runtimeElementAttribute.type, attr_name, attribute.element_enduser_name);

    if (runtimeElementAttribute.type !== RuntimeElementAttributeType.String && value === "") {
        return validatorMessageRenderer(validator.getLanguageId(), ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_VALUE_NOT_EMPTY, attr_name, attribute.element_enduser_name);
    }

    switch (runtimeElementAttribute.type) {
        case RuntimeElementAttributeType.Url: {
            if (isUrl(value)) {
                return undefined;
            } else {
                return error;
            }
        }

        case RuntimeElementAttributeType.Urls: {
            // TODO: handle array of values
            return undefined;
        }

        case RuntimeElementAttributeType.Strings: {
            // TODO: handle array of values
            return undefined;
        }

        case RuntimeElementAttributeType.FontSrc: {
            const extenstions: string[] = [".woff", ".woff2", ".ttf", ".otf", ".eot", ".svg"];
            if (!extenstions.some(ext => value.endsWith(ext))) {
                return error;
            }
            attribute.output_type = RuntimeElementAttributeOutputType.UrlWithFont;

            return undefined;
        }

        case RuntimeElementAttributeType.String: {
            if (value === "") {
                return undefined;
            }
            return error;
        }

        case RuntimeElementAttributeType.StringNoEmpty: {
            // TODO: it's better to remove "runtimeElementAttribute.type" from the error message
            return error;
        }

        case RuntimeElementAttributeType.Sizes: {
            // TODO: handle array of values
            return undefined;
        }

        case RuntimeElementAttributeType.Size: {
            return undefined;
        }

        case RuntimeElementAttributeType.PositiveInt: {
            if (isNaN(Number(value)) || Number(value) < 0) {
                return error;
            }
            return undefined;
        }

        case RuntimeElementAttributeType.IDs: {
            // TODO: handle array of values
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
            const coords: string[] = value.split(",");
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
                } else {
                    return undefined
                }
            }
            return error;
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
            const datePattern: RegExp = /^\d{4}-\d{2}-\d{2}$/;
            if (! datePattern.test(value) || isNaN(Date.parse(value))) {
                return error;
            }
            return undefined;
        }
    
        case RuntimeElementAttributeType.Time: {
            // Example: 12:00
            const timePattern: RegExp = /^([01]?[0-9]|2[0-3]):([0-5][0-9])$/;
            if (! timePattern.test(value) || isNaN(Date.parse(`1970-01-01T${value}:00Z`))) {
                return error;
            }
            return undefined;
        }
    
        case RuntimeElementAttributeType.Times: {
            // TODO: make sure it is array of valid times
        }
    
        case RuntimeElementAttributeType.DateTime: {
            // Example: 2021-01-01T12:00:00
            const dateTimePattern: RegExp = /^\d{4}-\d{2}-\d{2}T([01]?[0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9])$/;
            if (! dateTimePattern.test(value) || isNaN(Date.parse(value))) {
                return error;
            }
            return undefined;
        }
      
        case RuntimeElementAttributeType.NumberOr2Numbers: {
            const parts: string[] = value.split(" ");
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
