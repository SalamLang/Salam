import { Validator } from "./../validator";
import { validateBlock } from "./../statement/block";
import { AstBlock } from "../../../parser/parse/ast/block";
import { validateFunctionArguments } from "./function_arguments";
import { AstFunctionDeclaration } from "./../../../parser/parse/ast/function/function_declaration";

export function validateFunctionDeclaration(validator: Validator, parent_block: AstBlock, func: AstFunctionDeclaration): void {
    validateFunctionArguments(validator, func.args);
    validateBlock(validator, parent_block, func.body);
};
