import { RuntimeElement } from './element';
import { RuntimeElementAttribute } from './element_attribute';
import { RuntimeElementStyleState } from './element_style_state';
import { RuntimeElementAttributeStyle } from './element_attribute_style';

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

import { RuntimeStyleAccentColor } from './style/attribute/accent-color';
import { RuntimeStyleAll } from './style/attribute/all';
import { RuntimeStyleAnimationComposition } from './style/attribute/animation-composition';
import { RuntimeStyleAnimationDelay } from './style/attribute/animation-delay';
import { RuntimeStyleAnimationDirection } from './style/attribute/animation-direction';
import { RuntimeStyleAnimationDuration } from './style/attribute/animation-duration';
import { RuntimeStyleAnimationFillMode } from './style/attribute/animation-fill-mode';
import { RuntimeStyleAnimationIterationCount } from './style/attribute/animation-iteration-count';
import { RuntimeStyleAnimationName } from './style/attribute/animation-name';
import { RuntimeStyleAnimationPlayState } from './style/attribute/animation-play-state';
import { RuntimeStyleAnimationTimingFunction } from './style/attribute/animation-timing-function';
import { RuntimeStyleAppearance } from './style/attribute/appearance';
import { RuntimeStyleAspectRation } from './style/attribute/aspect-ratio';
import { RuntimeStyleBackdropFilter } from './style/attribute/backdrop-filter';
import { RuntimeStyleBackfaceVisibility } from './style/attribute/backface-visibility';
import { RuntimeStyleBackgroundColor } from './style/attribute/background-color';
import { RuntimeStyleBlockSize } from './style/attribute/block-size';
import { RuntimeStyleBottom } from './style/attribute/bottom';
import { RuntimeStyleCaptionSide } from './style/attribute/caption-side';
import { RuntimeStyleCaretColor } from './style/attribute/caret-color';
import { RuntimeStyleClear } from './style/attribute/clear';
import { RuntimeStyleColor } from './style/attribute/color';
import { RuntimeStyleFontSize } from './style/attribute/font-size';
import { RuntimeStyleTextAlign } from './style/attribute/text-align';

export const runtimeStyleAttributes: RuntimeElementAttributeStyle[] = [
    new RuntimeStyleAccentColor(),
    new RuntimeStyleAll(),
    new RuntimeStyleAnimationComposition(),
    new RuntimeStyleAnimationDelay(),
    new RuntimeStyleAnimationDirection(),
    new RuntimeStyleAnimationDuration(),
    new RuntimeStyleAnimationFillMode(),
    new RuntimeStyleAnimationIterationCount(),
    new RuntimeStyleAnimationName(),
    new RuntimeStyleAnimationPlayState(),
    new RuntimeStyleAnimationTimingFunction(),
    new RuntimeStyleAppearance(),
    new RuntimeStyleAspectRation(),
    new RuntimeStyleBackdropFilter(),
    new RuntimeStyleBackfaceVisibility(),
    new RuntimeStyleBackgroundColor(),
    new RuntimeStyleBlockSize(),
    new RuntimeStyleBottom(),
    new RuntimeStyleCaptionSide(),
    new RuntimeStyleCaretColor(),
    new RuntimeStyleClear(),
    new RuntimeStyleColor(),
    new RuntimeStyleFontSize(),
    new RuntimeStyleTextAlign(),
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
    new RuntimeElementStyleStateActive(),
    new RuntimeElementStyleStateBlank(),
    new RuntimeElementStyleStateChecked(),
    new RuntimeElementStyleStateDisabled(),
    new RuntimeElementStyleStateEmpty(),
    new RuntimeElementStyleStateEnabled(),
    new RuntimeElementStyleStateFirstChild(),
    new RuntimeElementStyleStateFirstOfType(),
    new RuntimeElementStyleStateFocusVisible(),
    new RuntimeElementStyleStateFocusWithin(),
    new RuntimeElementStyleStateFocus(),
    new RuntimeElementStyleStateGlobal(),
    new RuntimeElementStyleStateHover(),
    new RuntimeElementStyleStateInRange(),
    new RuntimeElementStyleStateLastChild(),
    new RuntimeElementStyleStateLastOfType(),
    new RuntimeElementStyleStateLink(),
    new RuntimeElementStyleStateNthChild(),
    new RuntimeElementStyleStateNthLastChild(),
    new RuntimeElementStyleStateNthLastOfType(),
    new RuntimeElementStyleStateNthOfType(),
    new RuntimeElementStyleStateOnlyChild(),
    new RuntimeElementStyleStateOnlyOfType(),
    new RuntimeElementStyleStateOptional(),
    new RuntimeElementStyleStateRoot(),
    new RuntimeElementStyleStateTarget(),
    new RuntimeElementStyleStateValid(),
    new RuntimeElementStyleStateVisited(),
];
