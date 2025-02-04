import { AstBlock } from "../../phase/parser/parse/ast/block";
import { AstType } from "../../phase/parser/parse/ast/expression/type";
import { AstFunctionArgument } from "../../phase/parser/parse/ast/function/function_argument";

export class RuntimeBlock {
    static generate(): AstBlock {
        const block: AstBlock = new AstBlock();

        const math_type: AstType = new AstType("package");
        const math_sin_type: AstType = AstType.createMemberFunction("sin", [new AstFunctionArgument("value", AstType.createFloat())], AstType.createFloat());
        math_type.addMember(math_sin_type);
        const math_cos_type: AstType = AstType.createMemberFunction("cos", [new AstFunctionArgument("value", AstType.createFloat())], AstType.createFloat());
        math_type.addMember(math_cos_type);
        const math_tan_type: AstType = AstType.createMemberFunction("tan", [new AstFunctionArgument("value", AstType.createFloat())], AstType.createFloat());
        math_type.addMember(math_tan_type);
        const math_creator_type: AstType = AstType.createMemberValue("creator", AstType.createString());
        math_type.addMember(math_creator_type);
        math_type.setSyetem();
        block.symbol_table.addSystemSymbol("math", math_type);

        const pi_type: AstType = AstType.createFloat();
        pi_type.setSyetem();
        // console.log(pi_type);
        block.symbol_table.addSystemSymbol("pi", pi_type);

        const version_type: AstType = AstType.createString();
        version_type.setSyetem();
        block.symbol_table.addSystemSymbol("version", version_type);

        return block;
    }
};
