import { MessageKeys, MessageStruct, GeneratorMessageKeys } from './../message';

export const faMessages: MessageStruct<GeneratorMessageKeys> = {
    [MessageKeys.GENERATOR_PREFIX]: "خطای تولید کننده: ",
    [MessageKeys.GENERATOR_CANNOT_OUTDENT_BELOW_ZERO]: "نمی‌توانید از صفر کمتر برود",
    [MessageKeys.GENERATOR_UNKNOWN_ELEMENT_TYPE]: "نوع عنصر ناشناخته: {0}",
    [MessageKeys.GENERATOR_INCLUDE_FILE_NOT_FOUND]: "فایل در دستور فراخوانی یافت نشد - \"{0}\"",
    [MessageKeys.GENERATOR_INCLUDE_FILE_READ_ERROR]: "خطا در خواندن فایل در دستور فراخوانی - \"{0}\"",
    [MessageKeys.GENERATOR_INCLUDE_FILE_PATH_NOT_PROVIDED]: "مسیر فایل در دستور فراخوانی ارائه نشده است",
};
