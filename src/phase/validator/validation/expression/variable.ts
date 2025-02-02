import { Validator } from "./../validator";
import { validateExpression } from "./expression";
import { AstBlock } from "../../../parser/parse/ast/block";
import { AstType } from "../../../parser/parse/ast/expression/type";
import { AstExpressionVariable } from "../../../parser/parse/ast/expression/variable";

export function validateExpressionVariable(validator: Validator, block: AstBlock, expr: AstExpressionVariable): void {
    validateExpression(validator, block, expr.left);

    if (expr.left.value_type === undefined) {
        return;
    }

    const name: string = expr.left.getString();
    const symbol_type: AstType | undefined = block.symbol_table.getSymbol(name);

    if (symbol_type === undefined && block.checkVariables === true) {
        validator.pushError(`Variable '${name}' is not defined`);
        return;
    }

    expr.value_type = symbol_type;
};
