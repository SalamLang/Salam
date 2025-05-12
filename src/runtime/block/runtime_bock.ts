import { AstBlock } from "../../phase/parser/parse/ast/block";
import { AstType } from "../../phase/parser/parse/ast/expression/type";
import { AstFunctionArgument } from "../../phase/parser/parse/ast/function/function_argument";

export class RuntimeBlock {
    static generate(): AstBlock {
        const block: AstBlock = new AstBlock();

        // const math_type: AstType = AstType.createPackage();
        // math_type.setSyetem();
        // const math_sin_type: AstType = AstType.createFunction("sin", "sin", [new AstFunctionArgument("value", AstType.createFloat())], AstType.createFloat());
        // math_sin_type.setSyetem();
        // math_type.addMember(math_sin_type);
        // const math_cos_type: AstType = AstType.createFunction("cos", "cos", [new AstFunctionArgument("value", AstType.createFloat())], AstType.createFloat());
        // math_cos_type.setSyetem();
        // math_type.addMember(math_cos_type);
        // const math_tan_type: AstType = AstType.createFunction("tan", "tan", [new AstFunctionArgument("value", AstType.createFloat())], AstType.createFloat());
        // math_tan_type.setSyetem();
        // math_type.addMember(math_tan_type);
        // const math_creator_type: AstType = AstType.createMemberValue("creator", AstType.createString());
        // math_type.addMember(math_creator_type);
        // math_type.setSyetem();
        // block.symbol_table.addSystemSymbol("math", math_type);

        // const pi_type: AstType = AstType.createFloat();
        // pi_type.setSyetem();
        // block.symbol_table.addSystemSymbol("pi", pi_type);

        // const version_type: AstType = AstType.createString();
        // version_type.setSyetem();
        // block.symbol_table.addSystemSymbol("version", version_type);

        // const int2str_type: AstType = AstType.createFunction("int2str", "int2str", [new AstFunctionArgument("value", AstType.createInt())], AstType.createString());
        // int2str_type.setSyetem();
        // block.symbol_table.addSystemSymbol("int2str", int2str_type);

        // const float2str_type: AstType = AstType.createFunction("float2str", "float2str", [new AstFunctionArgument("value", AstType.createFloat())], AstType.createString());
        // float2str_type.setSyetem();
        // block.symbol_table.addSystemSymbol("float2str", float2str_type);

        // const memory_alloc_type: AstType = AstType.createFunction("malloc", "malloc", [new AstFunctionArgument("size", AstType.createSize())], AstType.createPointer());
        // memory_alloc_type.setSyetem();
        // block.symbol_table.addSystemSymbol("malloc", memory_alloc_type);

        // const memory_realloc_type: AstType = AstType.createFunction("realloc", "realloc", [new AstFunctionArgument("pointer", AstType.createPointer()), new AstFunctionArgument("size", AstType.createSize())], AstType.createPointer());
        // memory_realloc_type.setSyetem();
        // block.symbol_table.addSystemSymbol("realloc", memory_realloc_type);

        // const free_type: AstType = AstType.createFunction("free", "free", [new AstFunctionArgument("pointer", AstType.createPointer())], AstType.createVoid());
        // free_type.setSyetem();
        // block.symbol_table.addSystemSymbol("free", free_type);

        // const strdup_type: AstType = AstType.createFunction("strdup", "strdup", [new AstFunctionArgument("value", AstType.createString())], AstType.createVoid());
        // strdup_type.setSyetem();
        // block.symbol_table.addSystemSymbol("strdup", strdup_type);

        // const strcpy_type: AstType = AstType.createFunction("strcpy", "strcpy", [new AstFunctionArgument("pointer", AstType.createPointer()), new AstFunctionArgument("value", AstType.createString())], AstType.createVoid());
        // strcpy_type.setSyetem();
        // block.symbol_table.addSystemSymbol("strcpy", strcpy_type);

        // const strcat_type: AstType = AstType.createFunction("strcat", "strcat", [new AstFunctionArgument("target", AstType.createString()), new AstFunctionArgument("append", AstType.createString())], AstType.createVoid());
        // strcat_type.setSyetem();
        // block.symbol_table.addSystemSymbol("strcat", strcat_type);

        return block;
    }
};
