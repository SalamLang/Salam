import { ValidatorMessageKeys } from './validator';

export const faMessages: Record<ValidatorMessageKeys, string> = {
    [ValidatorMessageKeys.VALIDATOR_PREFIX]: "خطا اعتبارسنجی: ",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_REQUIRED]: "ویژگی '{0}' برای عنصر '{1}' لازم است اما یافت نشد",
    [ValidatorMessageKeys.VALIDATOR_ELEMENT_NOT_VALID]: "عنصر '{0}' یک عنصر معتبر نیست",
    [ValidatorMessageKeys.VALIDATOR_ELEMENT_STYLE_STATE_NOT_VALID]: "وضعیت استایل عنصر '{0}' یک وضعیت استایل معتبر نیست",
    [ValidatorMessageKeys.VALIDATOR_ELEMENT_OR_STYLE_STATE_NOT_VALID]: "عنصر یا وضعیت استایل '{0}' یک عنصر یا وضعیت استایل معتبر نیست",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_RESERVED_VALUE_NOT_VALID]: "مقدار ویژگی '{1}' برای ویژگی '{1}' در عنصر '{2}' معتبر نیست",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_NOT_VALID]: "ویژگی '{0}' برای عنصر '{1}' معتبر نیست",
    [ValidatorMessageKeys.VALIDATOR_STYLE_ATTRIBUTE_NOT_VALID]: "ویژگی استایل '{0}' برای عنصر '{1}' معتبر نیست",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_VALUE_NOT_VALID]: "مقدار ویژگی '{0}' برای ویژگی '{1}' در عنصر '{2}' یک مقدار معتبر نیست",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_VALUE_NOT_EMPTY]: "مقدار ویژگی '{0}' در عنصر '{1}' نباید خالی باشد",
    [ValidatorMessageKeys.VALIDATOR_ATTRIBUTE_NOT_SUPPORTED]: "ویژگی '{0}' برای عنصر '{1}' پشتیبانی نمی شود",
    [ValidatorMessageKeys.VALIDATOR_SAVE_OUTPUT_ERROR]: "خطا در ذخیره درخت در فایل '{0}' - {1}",
};
