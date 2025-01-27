import { ValidatorMessageKeys, ValidatorMessageStruct } from './../message';

export const enMessages: ValidatorMessageStruct = {
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_REQUIRED]: "Attribute '{0}' is required for element '{1}}' but not found",
    [ValidatorMessageKeys.VALIDATOR_ELEMENT_NOT_VALID]: "Element '{0}' is not a valid element",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_RESERVED_VALUE_NOT_VALID]: "Attribute value '{0}' is not a valid value for attribute '{1}' in element '{2}'",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_NOT_VALID]: "Attribute '{0}' is not a valid attribute for element '{1}'",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_VALUE_NOT_VALID]: "Attribute value '{0}' is not a valid {1} for attribute '{2}' in element '{3}",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_VALUE_NOT_EMPTY]: "Attribute value '{0}' is not a valid {1} for attribute '{2} as it should not be empty in element '{3}",
};
