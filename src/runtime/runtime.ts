import { RuntimeElementWbr } from './element/wbr';
import { RuntimeElementVideo } from './element/video';
import { RuntimeElementVar } from './element/var';
import { RuntimeElementUl } from './element/ul';
import { RuntimeElementU } from './element/u';
import { RuntimeElementTt } from './element/tt';
import { RuntimeElementTrack } from './element/track';
import { RuntimeElementTr } from './element/tr';
import { RuntimeElementTime } from './element/time';
import { RuntimeElementThead } from './element/thead';
import { RuntimeElementTh } from './element/th';
import { RuntimeElementTfoot } from './element/tfoot';
import { RuntimeElementTextarea } from './element/textarea';
import { RuntimeElementTemplate } from './element/template';
import { RuntimeElementTbody } from './element/tbody';
import { RuntimeElementTable } from './element/table';
import { RuntimeElementSvg } from './element/svg';
import { RuntimeElementSup } from './element/sup';
import { RuntimeElementSummary } from './element/summary';
import { RuntimeElementSub } from './element/sub';
import { RuntimeElementSource } from './element/source';
import { RuntimeElementSection } from './element/section';
import { RuntimeElementSearch } from './element/search';
import { RuntimeElementSamp } from './element/samp';
import { RuntimeElementS, RuntimeElementScript } from './element/script';
import { RuntimeElementRuby } from './element/ruby';
import { RuntimeElementRt } from './element/rt';
import { RuntimeElementRp } from './element/rp';
import { RuntimeElementQ } from './element/q';
import { RuntimeElementProgress } from './element/progress';
import { RuntimeElementPre } from './element/pre';
import { RuntimeElementPicture } from './element/picture';
import { RuntimeElementParam } from './element/param';
import { RuntimeElementP } from './element/p';
import { RuntimeElementOutput } from './element/output';
import { RuntimeElementOption } from './element/option';
import { RuntimeElementOl } from './element/ol';
import { RuntimeElementNoScript } from './element/noscript';
import { RuntimeElementLink } from './element/link';
import { RuntimeElementLi } from './element/li';
import { RuntimeElementLegend } from './element/legend';
import { RuntimeElementLabel } from './element/label';
import { RuntimeElementKbd } from './element/kbd';
import { RuntimeElementIns } from './element/ins';
import { RuntimeElementInput } from './element/input';
import { RuntimeElementIframe } from './element/iframe';
import { RuntimeElementI } from './element/i';
import { RuntimeElementHr } from './element/hr';
import { RuntimeElementHgroup } from './element/hgroup';
import { RuntimeElementHeader } from './element/header';
import { RuntimeElementH6 } from './element/h6';
import { RuntimeElementH3 } from './element/h3';
import { RuntimeElementH2 } from './element/h2';
import { RuntimeElementH1 } from './element/h1';
import { RuntimeElementForm } from './element/form';
import { RuntimeElementFooter } from './element/footer';
import { RuntimeElementFigure } from './element/figure';
import { RuntimeElementFigcaption } from './element/figcaption';
import { RuntimeElementFieldset } from './element/fieldset';
import { RuntimeElementEm } from './element/em';
import { RuntimeElementDt } from './element/dt';
import { RuntimeElementDialog } from './element/dialog';
import { RuntimeElementDFN } from './element/dfn';
import { RuntimeElementDetails } from './element/details';
import { RuntimeElementDel } from './element/del';
import { RuntimeElementDD } from './element/dd';
import { RuntimeElementDatalist } from './element/datalist';
import { RuntimeElementData } from './element/data';
import { RuntimeElementColgroup } from './element/colgroup';
import { RuntimeElementCol } from './element/col';
import { RuntimeElementCode } from './element/code';
import { RuntimeElementCite } from './element/cite';
import { RuntimeElementCaption } from './element/caption';
import { RuntimeElementButton } from './element/button';
import { RuntimeElementBr } from './element/br';
import { RuntimeElementBlockquote } from './element/blockquote';
import { RuntimeElementBdo } from './element/bdo';
import { RuntimeElementBdi } from './element/bdi';
import { RuntimeElementBase } from './element/base';
import { RuntimeElementB } from './element/b';
import { RuntimeElementAudio } from './element/audio';
import { RuntimeElementAside } from './element/aside';
import { RuntimeElementArticle } from './element/article';
import { RuntimeElementArea } from './element/area';
import { RuntimeElementAddress } from './element/address';
import { RuntimeElementAbbr } from './element/abbr';
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
    new RuntimeElementInclude(),
    new RuntimeElementA(),
    new RuntimeElementAbbr(),
    new RuntimeElementAddress(),
    new RuntimeElementArea(),
    new RuntimeElementArticle(),
    new RuntimeElementAside(),
    new RuntimeElementAudio(),
    new RuntimeElementB(),
    new RuntimeElementBase(),
    new RuntimeElementBdi(),
    new RuntimeElementBdo(),
    new RuntimeElementBlockquote(),
    new RuntimeElementBr(),
    new RuntimeElementButton(),
    new RuntimeElementCanvas(),
    new RuntimeElementCaption(),
    new RuntimeElementCite(),
    new RuntimeElementCode(),
    new RuntimeElementCol(),
    new RuntimeElementColgroup(),
    new RuntimeElementData(),
    new RuntimeElementDatalist(),
    new RuntimeElementDD(),
    new RuntimeElementDel(),
    new RuntimeElementDetails(),
    new RuntimeElementDFN(),
    new RuntimeElementDialog(),
    new RuntimeElementDiv(),
    new RuntimeElementDt(),
    new RuntimeElementEm(),
    new RuntimeElementFieldset(),
    new RuntimeElementFigcaption(),
    new RuntimeElementFigure(),
    new RuntimeElementFooter(),
    new RuntimeElementForm(),
    new RuntimeElementH1(),
    new RuntimeElementH2(),
    new RuntimeElementH3(),
    new RuntimeElementH4(),
    new RuntimeElementH5(),
    new RuntimeElementH6(),
    new RuntimeElementHeader(),
    new RuntimeElementHgroup(),
    new RuntimeElementHr(),
    new RuntimeElementI(),
    new RuntimeElementIframe(),
    new RuntimeElementImg(),
    new RuntimeElementInput(),
    new RuntimeElementIns(),
    new RuntimeElementKbd(),
    new RuntimeElementLabel(),
    new RuntimeElementLegend(),
    new RuntimeElementLi(),
    new RuntimeElementLink(),
    new RuntimeElementNoScript(),
    new RuntimeElementOl(),
    new RuntimeElementOption(),
    new RuntimeElementOutput(),
    new RuntimeElementP(),
    new RuntimeElementParam(),
    new RuntimeElementPicture(),
    new RuntimeElementPre(),
    new RuntimeElementProgress(),
    new RuntimeElementQ(),
    new RuntimeElementRp(),
    new RuntimeElementRt(),
    new RuntimeElementRuby(),
    new RuntimeElementS(),
    new RuntimeElementSamp(),
    new RuntimeElementScript(),
    new RuntimeElementSearch(),
    new RuntimeElementSection(),
    new RuntimeElementSelect(),
    new RuntimeElementSmall(),
    new RuntimeElementSource(),
    new RuntimeElementStrong(),
    new RuntimeElementStyle(),
    new RuntimeElementSub(),
    new RuntimeElementSummary(),
    new RuntimeElementSup(),
    new RuntimeElementSvg(),
    new RuntimeElementTable(),
    new RuntimeElementTbody(),
    new RuntimeElementTemplate(),
    new RuntimeElementTextarea(),
    new RuntimeElementTfoot(),
    new RuntimeElementTh(),
    new RuntimeElementThead(),
    new RuntimeElementTime(),
    new RuntimeElementTr(),
    new RuntimeElementTrack(),
    new RuntimeElementTt(),
    new RuntimeElementU(),
    new RuntimeElementUl(),
    new RuntimeElementVar(),
    new RuntimeElementVideo(),
    new RuntimeElementWbr(),
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
import { RuntimeStyleFontFamily } from './style/attribute/font-family';
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
    new RuntimeStyleFontFamily(),
    new RuntimeStyleTextAlign(),
];

import { RuntimeGlobalAttributeContent } from './attribute/content';

export const runtimeGlobalMotherAttributes: RuntimeElementAttribute[] = [
    new RuntimeGlobalAttributeContent(),
];

export const runtimeGlobalSingleAttributes: RuntimeElementAttribute[] = [
];

import { RuntimeGlobalAttributeId } from './attribute/id';
import { RuntimeGlobalAttributeClass } from './attribute/class';
import { RuntimeGlobalAttributeRepeat } from './attribute/custom/repeat';

export const runtimeGlobalAttributes: RuntimeElementAttribute[] = [
    new RuntimeGlobalAttributeId(),
    new RuntimeGlobalAttributeClass(),
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
import { RuntimeElementCanvas } from './element/canvas';
import { RuntimeElementH4 } from './element/h4';
import { RuntimeElementH5 } from './element/h5';
import { RuntimeElementImg } from './element/img';
import { RuntimeElementSelect } from './element/select';
import { RuntimeElementSmall } from './element/small';
import { RuntimeElementStrong } from './element/strong';
import { RuntimeElementStyle } from './element/style';

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

import { RuntimeStyleElementFont } from './style/element/font';

export const runtimeStyleElements: RuntimeElement[] = [
    new RuntimeStyleElementFont(),
];
