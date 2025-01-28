import { ValidatorMessageKeys } from './validator';

export const enMessages: Record<ValidatorMessageKeys, string> = {
    [ValidatorMessageKeys.VALIDATOR_PREFIX]: "Validation error: ",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_REQUIRED]: "Attribute '{0}' is required for element '{1}}' but not found",
    [ValidatorMessageKeys.VALIDATOR_ELEMENT_NOT_VALID]: "Element '{0}' is not a valid element",
    [ValidatorMessageKeys.VALIDATOR_ELEMENT_STYLE_STATE_NOT_VALID]: "Element style state '{0}' is not a valid element style state",
    [ValidatorMessageKeys.VALIDATOR_ELEMENT_OR_STYLE_STATE_NOT_VALID]: "Element or style state '{0}' is not a valid element or style state",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_RESERVED_VALUE_NOT_VALID]: "Attribute value '{0}' is not a valid value for attribute '{1}' in element '{2}'",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_NOT_VALID]: "Attribute '{0}' is not a valid attribute for element '{1}'",
    [ValidatorMessageKeys.VALIDATOR_STYLE_ATTRIBUTE_NOT_VALID]: "Style attribute '{0}' is not valid for element '{1}'",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_VALUE_NOT_VALID]: "Attribute value '{0}' is not a valid {1} for attribute '{2}' in element '{3}",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_VALUE_NOT_EMPTY]: "Attribute value '{0}' is not a valid {1} for attribute '{2} as it should not be empty in element '{3}",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_NOT_SUPPORTED]: "Attribute '{0}' is not supported for element '{1}'",
};
