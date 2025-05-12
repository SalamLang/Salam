import { Validator } from "../validator";
import { AstBlock } from "../../../parser/parse/ast/block";
import { validateFunctionParameter } from "./function_parameter";
import { AstFunctionParameter } from "../../../parser/parse/ast/function/function_parameter";

export function validateFunctionParameters(validator: Validator, block: AstBlock, attributes: AstFunctionParameter[]): void {
    for (const attribute of attributes) {
        validateFunctionParameter(validator, block, attribute);
    }
};
