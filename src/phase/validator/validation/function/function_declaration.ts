import { Validator } from "./../validator";
import { validateBlock } from "./../statement/block";
import { AstBlock } from "../../../parser/parse/ast/block";
import { validateFunctionArguments } from "./function_arguments";
import { AstFunctionDeclaration } from "./../../../parser/parse/ast/function/function_declaration";
import { AstType } from "../../../parser/parse/ast/expression/type";

export function validateFunctionDeclaration(validator: Validator, parent_block: AstBlock, func: AstFunctionDeclaration): void {
    const table_func: AstType | undefined = parent_block.lookUp(func.name);
    if (table_func !== undefined) {
        if (table_func.isFunction) {
            validator.pushError("Another function with the same name is already defiend!");
        } else {
            validator.pushError("Another variable with the same name is already defiend!");
        }
        return;
    }

    validateFunctionArguments(validator, parent_block, func.args);

    for (const argument of func.args) {
        parent_block.symbol_table.addSymbol(argument.name, argument.value_type);
    }

    validateBlock(validator, parent_block, func.body);

    const func_type: AstType = AstType.createFunction(func.name, func.args, func.return_type);
    parent_block.symbol_table.addSymbol(func.name, func_type);
};
