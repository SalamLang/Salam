import { RuntimeElementLayout } from './element/custom/layout';
import { RuntimeElementA } from './element/a';

export const runtimeElements = [
    new RuntimeElementLayout(),
    new RuntimeElementA(),
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
