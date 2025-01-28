import { RuntimeElement } from './element';
import { RuntimeElementAttribute } from './element_attribute';

import { RuntimeElementA } from './element/a';
import { RuntimeElementDiv } from './element/div';
import { RuntimeElementLayout } from './element/custom/layout';
import { RuntimeElementInclude } from './element/custom/include';

export const runtimeElements: RuntimeElement[] = [
    new RuntimeElementLayout(),
    new RuntimeElementA(),
    new RuntimeElementInclude(),
    new RuntimeElementDiv(),
];

import { RuntimeStyleColor } from './style/attribute/color';
import { RuntimeStyleFontSize } from './style/attribute/font-size';
import { RuntimeStyleTextAlign } from './style/attribute/text-align';
import { RuntimeStyleBackgroundColor } from './style/attribute/background-color';

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

import { RuntimeElementStyleStateActive } from './style/state/active';
import { RuntimeElementStyleStateBlank } from './style/state/blank';
import { RuntimeElementStyleStateChecked } from './style/state/checked';
import { RuntimeElementStyleStateDisabled } from './style/state/disabled';
import { RuntimeElementStyleStateEmpty } from './style/state/empty';
import { RuntimeElementStyleStateEnabled } from './style/state/enabled';
import { RuntimeElementStyleStateFirstChild } from './style/state/first-child';
import { RuntimeElementStyleStateFirstOfType } from './style/state/first-of-type';
import { RuntimeElementStyleStateFocusVisible } from './style/state/focus-visible';
import { RuntimeElementStyleStateFocusWithin } from './style/state/focus-within';
import { RuntimeElementStyleStateFocus } from './style/state/focus';
import { RuntimeElementStyleStateGlobal } from './style/state/global';
import { RuntimeElementStyleStateHover } from './style/state/hover';
import { RuntimeElementStyleStateInRange } from './style/state/in-range';
import { RuntimeElementStyleStateLastChild } from './style/state/last-child';
import { RuntimeElementStyleStateLastOfType } from './style/state/last-of-type';
import { RuntimeElementStyleStateLink } from './style/state/link';
import { RuntimeElementStyleStateNthChild } from './style/state/nth-child';
import { RuntimeElementStyleStateNthLastChild } from './style/state/nth-last-child';
import { RuntimeElementStyleStateNthLastOfType } from './style/state/nth-last-of-type';
import { RuntimeElementStyleStateNthOfType } from './style/state/nth-of-type';
import { RuntimeElementStyleStateOnlyChild } from './style/state/only-child';
import { RuntimeElementStyleStateOnlyOfType } from './style/state/only-of-type';
import { RuntimeElementStyleStateOptional } from './style/state/optional';
import { RuntimeElementStyleStateRoot } from './style/state/root';
import { RuntimeElementStyleStateTarget } from './style/state/target';
import { RuntimeElementStyleStateValid } from './style/state/valid';
import { RuntimeElementStyleStateVisited } from './style/state/visited';

export const runtimeStyleStates: RuntimeElementStyleState[] = [

];
