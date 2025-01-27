import { MessageKeys, MessageStruct, ValidatorMessageKeys } from './../message';

export const faMessages: MessageStruct<ValidatorMessageKeys> = {
    [MessageKeys.VALIDATOR_PREFIX]: "خطا اعتبارسنجی: ",
    [MessageKeys.VALIDATOR_ATTRIBUTE_REQUIRED]: "ویژگی '{0}' برای عنصر '{1}' لازم است اما یافت نشد",
    [MessageKeys.VALIDATOR_ELEMENT_NOT_VALID]: "عنصر '{0}' یک عنصر معتبر نیست",
    [MessageKeys.VALIDATOR_ATTRIBUTE_RESERVED_VALUE_NOT_VALID]: "مقدار ویژگی '{1}' برای ویژگی '{1}' در عنصر '{2}' معتبر نیست",
    [MessageKeys.VALIDATOR_ATTRIBUTE_NOT_VALID]: "ویژگی '{0}' برای عنصر '{0}' معتبر نیست",
    [MessageKeys.VALIDATOR_ATTRIBUTE_VALUE_NOT_VALID]: "مقدار ویژگی '{0}' برای ویژگی '{1}' در عنصر '{2}' یک مقدار معتبر نیست",
    [MessageKeys.VALIDATOR_ATTRIBUTE_VALUE_NOT_EMPTY]: "مقدار ویژگی '{0}' برای ویژگی '{1}' در عنصر '{2}' نباید خالی باشد",
};
