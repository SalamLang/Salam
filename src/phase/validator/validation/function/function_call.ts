import { Validator } from "../validator";
import { AstBlock } from "../../../parser/parse/ast/block";
import { validateExpression } from "../expression/expression";
import { validateFunctionParameters } from "./function_parameters";
import { AstExpressionFunctionCall } from "../../../parser/parse/ast/function/function_call";
import { AstExpressionBinary } from "../../../parser/parse/ast/expression/binary";
import { AstType } from "../../../parser/parse/ast/expression/type";
import { AstFunctionParameter } from "../../../parser/parse/ast/function/function_parameter";
import { AstFunctionArgument } from "../../../parser/parse/ast/function/function_argument";

export function validateExpressionFunctionCall(validator: Validator, block: AstBlock, node: AstExpressionFunctionCall) {
    validateExpression(validator, block, node.left);

    const left_type: AstType | undefined = node.left.value_type?.getType();
    if (left_type === undefined) {
        return;
    }

    if (left_type.isFunction) {
        validator.pushError("Cannot call an invalid value which is not a real function!");
        return;
    }

    let enduser_name: string = "";
    if (node.left.type === "ExpressionBinary") {
        node.generated_value = (node.left as AstExpressionBinary).generated_value;
        enduser_name = (node.left as AstExpressionBinary).getString();
    } else if (node.left.type === "ExpressionVariable" && node.left.value_type !== undefined) {
        node.generated_value = node.left.value_type.func_name;
        enduser_name = node.left.value_type.getString();
    } else {
        validator.pushError("Cannot detect function name in your expression function call");
    }

    validateFunctionParameters(validator, block, node.parameters);

    // if (node.generated_value !== undefined && node.left.value_type !== undefined && node.left.value_type.is_system) {
    if (node.generated_value !== undefined && node.left.value_type !== undefined) {
        // TODO: EXTERN
        // validator.pushExtendedFunction(node.generated_value, node.left.value_type);

        const func_args: AstFunctionArgument[] = node.left.value_type.func_args;
        const user_func_args: AstFunctionParameter[] = node.parameters;
        const func_args_length: number = func_args.length;
        const user_func_args_length: number = user_func_args.length;

        // TODO: handling __VA_ARGS__

        if (func_args_length !== user_func_args_length) {
            validator.pushError(`Number of arguments passed to ${enduser_name} dont match with the definition of the functions, we expect ${func_args_length} but you passed ${user_func_args_length}`);
        }
        else if (func_args_length === user_func_args_length) {
            for (let i = 0; i < func_args_length; i++) {
                if (! user_func_args[i].value.value_type?.castablle(func_args[i].value_type)) {
                    validator.pushError("Value type does not match argument type in '"+ enduser_name +"' function, cannot save '" + user_func_args[i].value?.value_type?.getString() + "' as '" + func_args[i].value_type.getString() + "' type.");
                    return;
                }
            }
        }
    }

    node.value_type = left_type;
};
