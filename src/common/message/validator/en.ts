import { ValidatorMessageKeys, ValidatorMessageStruct } from './../message';

export const enMessages: ValidatorMessageStruct = {
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_REQUIRED]: "Attribute '{{attribute}}' is required for element '{{element}}' but not found",
    [ValidatorMessageKeys.VALIDATOR_ELEMENT_NOT_VALID]: "Element '{{element}}' is not a valid element",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_RESERVED_VALUE_NOT_VALID]: "Attribute value '{{value}}' is not a valid value for attribute '{{attribute}}' in element '{{element}}'",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_NOT_VALID]: "Attribute '{{attribute}}' is not a valid attribute for element '{{element}}'",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_VALUE_NOT_VALID]: "Attribute value '{{value}}' is not a valid {{type}} for attribute '{{attribute}}' in element '{{element}}",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_VALUE_NOT_EMPTY]: "Attribute value '{{value}}' is not a valid {{type}} for attribute '{{attribute}} as it should not be empty in element '{{element}}",
};
