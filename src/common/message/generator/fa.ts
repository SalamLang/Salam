import { GeneratorMessageKeys } from './generator';

export const faMessages: Record<GeneratorMessageKeys, string> = {
    [GeneratorMessageKeys.GENERATOR_PREFIX]: "خطای تولید کننده: ",
    [GeneratorMessageKeys.GENERATOR_CANNOT_OUTDENT_BELOW_ZERO]: "نمی‌توانید از صفر کمتر برود",
    [GeneratorMessageKeys.GENERATOR_UNKNOWN_ELEMENT_TYPE]: "نوع عنصر ناشناخته: {0}",
    [GeneratorMessageKeys.GENERATOR_INCLUDE_FILE_NOT_FOUND]: "فایل در دستور فراخوانی یافت نشد - \"{0}\"",
    [GeneratorMessageKeys.GENERATOR_INCLUDE_FILE_READ_ERROR]: "خطا در خواندن فایل در دستور فراخوانی - \"{0}\"",
    [GeneratorMessageKeys.GENERATOR_INCLUDE_FILE_PATH_NOT_PROVIDED]: "مسیر فایل در دستور فراخوانی ارائه نشده است",
    [GeneratorMessageKeys.GENERATOR_INCLUDE_HTTP_ERROR_HTTP_STATUS]: "خطا در فراخوانی از نشانی اینترنتی - \"{0}\" - کد وضعیت HTTP: {1}",
    [GeneratorMessageKeys.GENERATOR_INCLUDE_HTTP_ERROR]: "خطا در فراخوانی از نشانی اینترنتی - \"{0}\"",
};
