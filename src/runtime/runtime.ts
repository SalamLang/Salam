import { RuntimeElementAttribute } from './element_attribute';

import { RuntimeElementA } from './element/a';
import { RuntimeElementInclude } from './element/custom/include';
import { RuntimeElementLayout } from './element/custom/layout';
import { RuntimeElementDiv } from './element/div';

export const runtimeElements = [
    new RuntimeElementLayout(),
    new RuntimeElementA(),
    new RuntimeElementInclude(),
    new RuntimeElementDiv(),
];

import { RuntimeStyleColor } from './style/color';
import { RuntimeStyleFontSize } from './style/font-size';
import { RuntimeStyleTextAlign } from './style/text-align';
import { RuntimeStyleBackgroundColor } from './style/background-color';

export const runtimeStyleAttributes = [
    new RuntimeStyleColor(),
    new RuntimeStyleFontSize(),
    new RuntimeStyleTextAlign(),
    new RuntimeStyleBackgroundColor(),
];

import { RuntimeGlobalAttributeContent } from './attribute/content';

export const runtimeGlobalMotherAttributes: RuntimeElementAttribute[] = [
    new RuntimeGlobalAttributeContent(),
];

export const runtimeGlobalSingleAttributes: RuntimeElementAttribute[] = [
];

import { RuntimeGlobalAttributeRepeat } from './attribute/custom/repeat';

export const runtimeGlobalAttributes: RuntimeElementAttribute[] = [
    new RuntimeGlobalAttributeRepeat(),
];
