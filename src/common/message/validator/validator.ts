import { faMessages } from './fa';
import { enMessages } from './en';
import { MessageStruct } from './../message';
import { MultiLanguageMessages } from './../message';
import { LanguageID } from './../../language/language';

export const validatorMessages: MultiLanguageMessages<MessageStruct<object>> = {
    [LanguageID.LanguageEnglish]: enMessages,
    [LanguageID.LanguagePersian]: faMessages
};
