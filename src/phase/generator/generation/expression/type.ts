import { Generator } from './../generator';
import { AstType } from '../../../parser/parse/ast/expression/type';

export function generateTypePrimary(type: AstType): string {
    if (type.type_kind === "void") {
        return "void";
    }
    else if (type.type_kind === "int") {
        return "int";
    }
    else if (type.type_kind === "float") {
        return "float";
    }
    else if (type.type_kind === "bool") {
        return "bool";
    }
    else if (type.type_kind === "string") {
        return "char*";
    }
    else if (type.type_kind === "char") {
        return "char";
    }
};

export function generateType(generator: Generator, type: AstType): string {
    return generateTypePrimary(type);

    let result: string = "Type_" + type.type_kind;
    if (type.is_pointer) {
        result += "_Pointer";
    }
    if (type.is_reference) {
        result += "_Reference";
    }
    if (type.is_array) {
        result += "_Array";
    }
    return result;
};
