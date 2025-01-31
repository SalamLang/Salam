import { Generator } from "./../generator";
import { generateLayoutAttribute } from "./attribute";
import { AstLayoutElement } from "./../../../parser/parse/ast/layout/element";
import { AstLayoutAttribute } from '../../../parser/parse/ast/layout/attribute';
import { AstExpression } from './../../../parser/parse/ast/expression/expression';
import { AstLayoutAttributeType } from '../../../parser/parse/ast/layout/attribute_type';
import { RuntimeGlobalAttributeClass } from "../../../../runtime/attribute/class";
import { AstExpressionLiteral } from "../../../parser/parse/ast/expression/literal";
import { TokenValueType } from "../../../lexer/tokenizer/type";

export function generateLayoutAttributes(generator: Generator, element: AstLayoutElement): string {
    let htmlResult: string = "";
    let styleResult: string = "";

    // Check style attributes
    for (const attribute of element.styles.items) {
        const generatedAttribute = generateLayoutAttribute(generator, attribute);
        if (generatedAttribute.length > 0) {
            styleResult += styleResult.length > 0 ? `;${generatedAttribute}` : generatedAttribute;
        }
    }

    if (styleResult.length > 0 && element.built_in_selector !== undefined) {
        const element_selector: string = "." + element.built_in_selector;
        generator.pushStyle(`${element_selector} {${styleResult}}`);

        // Check if "class" attribute exists
        const attribute_class: AstLayoutAttribute | undefined = element.attributes.getByGenerateName("class");
        if (attribute_class !== undefined) {
            const new_class: string = attribute_class.value.getString() + " " + element.built_in_selector;
            attribute_class.value = new AstExpressionLiteral(new_class.trim(), TokenValueType.TOKEN_STRING);
        } else {
            // Append and add a new attribute
            const class_runtime = new RuntimeGlobalAttributeClass();
            const class_attribute_key: string | undefined = class_runtime.getText(generator.getLanguageId())?.[0];
            if (class_attribute_key) {
                const class_attribute_value: AstExpression = new AstExpression(element.built_in_selector);
                const class_attribute: AstLayoutAttribute = new AstLayoutAttribute(element.enduser_name, class_attribute_key, class_attribute_value, AstLayoutAttributeType.Normal);
                class_attribute.generate_name = class_runtime.generate_name;
                element.attributes.push(class_attribute);
            }
        }
    }

    // Check layout attributes
    for (const attribute of element.attributes.items) {
        const generatedAttribute = generateLayoutAttribute(generator, attribute);
        if (generatedAttribute.length > 0) {
            htmlResult += htmlResult.length > 0 ? ` ${generatedAttribute}` : generatedAttribute;
        }
    }

    return htmlResult;
};
