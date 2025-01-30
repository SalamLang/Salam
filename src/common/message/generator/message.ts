import { faMessages } from './fa';
import { enMessages } from './en';
import { GeneratorMessageKeys } from './generator';
import { MultiLanguageMessages } from './../message';
import { LanguageID } from './../../language/language';

export const generatorMessages: MultiLanguageMessages<GeneratorMessageKeys> = {
    [LanguageID.LanguageEnglish]: enMessages,
    [LanguageID.LanguagePersian]: faMessages
};
