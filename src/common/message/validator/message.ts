import { faMessages } from './fa';
import { enMessages } from './en';
import { ValidatorMessageKeys } from './validator';
import { MultiLanguageMessages } from './../message';
import { LanguageID } from './../../language/language';

export const validatorMessages: MultiLanguageMessages<ValidatorMessageKeys> = {
    [LanguageID.LanguageEnglish]: enMessages,
    [LanguageID.LanguagePersian]: faMessages
};
