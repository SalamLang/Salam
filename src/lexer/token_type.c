#include "token_type.h"

char *token_type_stringify(token_t *token) {
    if (!token) {
        return "Invalid token";
    }

    switch (token->type) {
        // Values
        case TOKEN_STRING: return "String";
        case TOKEN_IDENTIFIER: return "Identifier";
        case TOKEN_BOOLEAN: return "Boolean";
        case TOKEN_NUMBER_FLOAT: return "Number (Float)";
        case TOKEN_NUMBER_INT: return "Number (Integer)";
        case TOKEN_BOOL_TRUE: return "Boolean (True)";
        case TOKEN_BOOL_FALSE: return "Boolean (False)";
        
        // Words
        case TOKEN_IF: return "If";
        case TOKEN_ELSE: return "Else";
        case TOKEN_PRINT: return "Print";
        case TOKEN_FN: return "Function";
        case TOKEN_FOR: return "For";
        case TOKEN_WHILE: return "While";
        case TOKEN_REPEAT: return "Repeat";
        case TOKEN_DO: return "Do";
        case TOKEN_RET: return "Return";
        case TOKEN_BREAK: return "Break";
        case TOKEN_CONOTINUE: return "Continue";
        case TOKEN_LAYOUT: return "Layout";
        case TOKEN_BLOCK_BEGIN: return "Block Begin";
        case TOKEN_BLOCK_END: return "Block End";
        
        // Operators
        case TOKEN_NOT_EQUAL: return "Not Equal";
        case TOKEN_EQUAL: return "Equal";
        case TOKEN_AND_AND: return "Logical And";
        case TOKEN_OR_OR: return "Logical Or";
        case TOKEN_LESS_EQUAL: return "Less or Equal";
        case TOKEN_GREATER_EQUAL: return "Greater or Equal";
        case TOKEN_INCREMENT: return "Increment";
        case TOKEN_DECREMENT: return "Decrement";
        case TOKEN_SHIFT_LEFT: return "Shift Left";
        case TOKEN_SHIFT_RIGHT: return "Shift Right";
        case TOKEN_SHIFT_RIGHT_ASSIGN: return "Shift Right Assign";
        case TOKEN_SHIFT_LEFT_ASSIGN: return "Shift Left Assign";
        case TOKEN_LEFT_BRACE: return "Left Brace";
        case TOKEN_RIGHT_BRACE: return "Right Brace";
        case TOKEN_LEFT_BRACKET: return "Left Bracket";
        case TOKEN_RIGHT_BRACKET: return "Right Bracket";
        case TOKEN_COLON: return "Colon";
        case TOKEN_COMMA: return "Comma";
        case TOKEN_LEFT_PAREN: return "Left Parenthesis";
        case TOKEN_RIGHT_PAREN: return "Right Parenthesis";
        case TOKEN_PLUS: return "Plus";
        case TOKEN_MINUS: return "Minus";
        case TOKEN_MULTIPLY: return "Multiply";
        case TOKEN_DIVIDE: return "Divide";
        case TOKEN_MOD: return "Modulo";
        case TOKEN_POWER: return "Power";
        case TOKEN_ASSIGN: return "Assign";
        case TOKEN_LESS: return "Less Than";
        case TOKEN_GREATER: return "Greater Than";
        case TOKEN_NOT: return "Not";
        case TOKEN_AND_BIT: return "Bitwise And";
        case TOKEN_OR_BIT: return "Bitwise Or";
        case TOKEN_XOR_BIT: return "Bitwise Xor";
        
        // Others
        case TOKEN_ERROR: return "Error";
        case TOKEN_EOF: return "End of File";
    }
}
