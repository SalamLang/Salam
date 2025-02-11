import { Validator } from "../validator";
import { AstFunctionParameter } from "../../../parser/parse/ast/function/function_parameter";
import { validateExpression } from "../expression/expression";
import { AstBlock } from "../../../parser/parse/ast/block";

export function validateFunctionParameter(validator: Validator, block: AstBlock, parameter: AstFunctionParameter): void {
    validateExpression(validator, block, parameter.value);
};
