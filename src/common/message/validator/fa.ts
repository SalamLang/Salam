import { ValidatorMessageKeys, ValidatorMessageStruct } from '../message';

export const faMessages: ValidatorMessageStruct = {
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_REQUIRED]: "ویژگی '{{attribute}}' برای عنصر '{{element}}' لازم است اما یافت نشد",
    [ValidatorMessageKeys.VALIDATOR_ELEMENT_NOT_VALID]: "عنصر '{{element}}' یک عنصر معتبر نیست",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_RESERVED_VALUE_NOT_VALID]: "مقدار ویژگی '{{value}}' برای ویژگی '{{attribute}}' در عنصر '{{element}}' معتبر نیست",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_NOT_VALID]: "ویژگی '{{attribute}}' برای عنصر '{{element}}' معتبر نیست",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_VALUE_NOT_VALID]: "مقدار ویژگی '{{value}}' برای ویژگی '{{attribute}}' در عنصر '{{element}}' یک مقدار معتبر نیست",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_VALUE_NOT_EMPTY]: "مقدار ویژگی '{{value}}' برای ویژگی '{{attribute}}' در عنصر '{{element}}' نباید خالی باشد",
};
