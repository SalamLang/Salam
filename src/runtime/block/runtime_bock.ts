import { AstBlock } from "../../phase/parser/parse/ast/block";
import { AstType } from "../../phase/parser/parse/ast/expression/type";
import { AstFunctionArgument } from "../../phase/parser/parse/ast/function/function_argument";

export class RuntimeBlock {
    static generate(): AstBlock {
        const block: AstBlock = new AstBlock();

        const math_type: AstType = new AstType("package");
        const math_sin_type: AstType = AstType.createFunction("sin", [new AstFunctionArgument("value", AstType.createFloat())], AstType.createFloat());
        math_type.addMemberFunction(math_sin_type);
        const math_cos_type: AstType = AstType.createFunction("cos", [new AstFunctionArgument("value", AstType.createFloat())], AstType.createFloat());
        math_type.addMemberFunction(math_cos_type);
        const math_tan_type: AstType = AstType.createFunction("tan", [new AstFunctionArgument("value", AstType.createFloat())], AstType.createFloat());
        math_type.addMemberFunction(math_tan_type);
        block.symbol_table.addSymbol("math", math_type);

        const pi_type: AstType = AstType.createFloat();
        block.symbol_table.addSymbol("pi", pi_type);

        const version_type: AstType = AstType.createString();
        block.symbol_table.addSymbol("version", version_type);

        return block;
    }
};
