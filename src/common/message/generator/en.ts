import { GeneratorMessageKeys } from './generator';

export const enMessages: Record<GeneratorMessageKeys, string> = {
    [GeneratorMessageKeys.GENERATOR_PREFIX]: "Generator error: ",
    [GeneratorMessageKeys.GENERATOR_CANNOT_OUTDENT_BELOW_ZERO]: "Cannot outdent below 0",
    [GeneratorMessageKeys.GENERATOR_UNKNOWN_ELEMENT_TYPE]: "Unknown element type: {0}",
    [GeneratorMessageKeys.GENERATOR_INCLUDE_FILE_NOT_FOUND]: "File not found in include command - \"{0}\"",
    [GeneratorMessageKeys.GENERATOR_INCLUDE_FILE_READ_ERROR]: "Error reading file in include command - \"{0}\"",
    [GeneratorMessageKeys.GENERATOR_INCLUDE_FILE_PATH_NOT_PROVIDED]: "File path not provided in include command",
};
