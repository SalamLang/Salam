import { Validator } from "./validator";
import { AstBlock } from "../../parser/parse/ast/block";
import { AstExtern } from "../../parser/parse/ast/extern";
import { AstType } from "../../parser/parse/ast/expression/type";
import { AstExternType } from "../../parser/parse/ast/extern_type";
import { validateFunctionArguments } from "./function/function_arguments";

export function validateAllExterns(validator: Validator) {
    Object.keys(validator.ast.externs).forEach((key) => {
        const typedKey = key as AstExternType;
        const externsArray: AstExtern[] = validator.ast.externs[typedKey];
        externsArray.forEach((node: AstExtern) => {
            validateExtern(validator, validator.ast.block, node);
        });
    });
};

export function validateExtern(validator: Validator, parent_block: AstBlock, extern: AstExtern): void {
    const table_func: AstType | undefined = parent_block.lookUp(extern.name);
    if (table_func !== undefined) {
        if (table_func.isFunction) {
            validator.pushError("Another function with the same name is already defiend!");
        } else {
            validator.pushError("Another variable with the same name is already defiend!");
        }
        return;
    }

    if (extern.args !== undefined) {
        validateFunctionArguments(validator, parent_block, extern.args);

        for (const argument of extern.args) {
            parent_block.symbol_table.addSymbol(argument.name, argument.value_type);
        }
    }

    if (extern.kind === AstExternType.EXTERN_VAR && extern.return_type) {
        parent_block.symbol_table.addSymbol(extern.name, extern.return_type);
    } else if (extern.kind === AstExternType.EXTERN_FN && extern.args && extern.return_type) {
        const extern_type: AstType = AstType.createFunction(extern.name, extern.generate_name, extern.args, extern.return_type);
        parent_block.symbol_table.addSymbol(extern.name, extern_type);
    }
};
