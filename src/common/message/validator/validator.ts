import { faMessages } from './fa';
import { enMessages } from './en';
import { ValidatorMultiLanguageMessages } from './../message';
import { LanguageID } from './../../language/language';

export const validatorMessages: ValidatorMultiLanguageMessages = {
    [LanguageID.LanguageEnglish]: enMessages,
    [LanguageID.LanguagePersian]: faMessages
};
