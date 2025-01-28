import { ValidatorMessageKeys } from './validator';

export const faMessages: Record<ValidatorMessageKeys, string> = {
    [ValidatorMessageKeys.VALIDATOR_PREFIX]: "خطا اعتبارسنجی: ",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_REQUIRED]: "ویژگی '{0}' برای عنصر '{1}' لازم است اما یافت نشد",
    [ValidatorMessageKeys.VALIDATOR_ELEMENT_NOT_VALID]: "عنصر '{0}' یک عنصر معتبر نیست",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_RESERVED_VALUE_NOT_VALID]: "مقدار ویژگی '{1}' برای ویژگی '{1}' در عنصر '{2}' معتبر نیست",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_NOT_VALID]: "ویژگی '{0}' برای عنصر '{0}' معتبر نیست",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_VALUE_NOT_VALID]: "مقدار ویژگی '{0}' برای ویژگی '{1}' در عنصر '{2}' یک مقدار معتبر نیست",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_VALUE_NOT_EMPTY]: "مقدار ویژگی '{0}' برای ویژگی '{1}' در عنصر '{2}' نباید خالی باشد",
};
