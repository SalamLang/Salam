import { Validator } from "./validator";
import { validateAllExterns, validateExtern } from "./extern";
import { validateLayout } from "./layout/layout";
import { AstExtern } from "../../parser/parse/ast/extern";
import { validateFunctionDeclaration } from "./function/function_declaration";
import { AstFunctionDeclaration } from "./../../parser/parse/ast/function/function_declaration";

export function validate(validator: Validator): void {
    validateAllExterns(validator);

    validator.ast.functions.forEach((node: AstFunctionDeclaration) => {
        validateFunctionDeclaration(validator, validator.ast.block, node as AstFunctionDeclaration);
    });

    // const int2str_type: AstType = AstType.createFunction("int2str", "int2str", [new AstFunctionArgument("value", AstType.createInt())], AstType.createString());
    // int2str_type.setSyetem();
    // block.symbol_table.addSystemSymbol("int2str", int2str_type);

    if (validator.ast.layout !== undefined) {
        validateLayout(validator, validator.ast.block, validator.ast.layout);
    }
};
