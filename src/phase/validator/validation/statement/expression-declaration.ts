import { Validator } from "./../validator";
import { validateType } from "../expression/type";
import { AstBlock } from "../../../parser/parse/ast/block";
import { validateExpression } from "../expression/expression";
import { AstStatementExpressionDeclaration } from "../../../parser/parse/ast/statement/expression_declaration";

export function validateStatementExpressionDeclaration(validator: Validator, block: AstBlock, stmt: AstStatementExpressionDeclaration): void {
    validateType(validator, stmt.value_type);

    if (stmt.value_type.type_kind === "Void") {
        validator.pushError("Cannot declare a variable of type 'void'.");
    }

    if (stmt.value !== undefined) {
        validateExpression(validator, block, stmt.value);
    }

    if (block.symbol_table.hasSymbol(stmt.name)) {
        validator.pushError(`Variable '${stmt.name}' already declared in this scope.`);
    } else {
        block.symbol_table.addSymbol(stmt.name, stmt.value_type);
    }
};
