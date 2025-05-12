import { TokenOperatorType, TokenType } from "../../../lexer/tokenizer/type";

export interface binding_power {
    left_power: number;
    right_power: number;
};

export function LeftAssociative(priority: number): binding_power {
    return { left_power: (priority - 1), right_power: priority };
}

export function RightAssociative(priority: number): binding_power {
    return { left_power: (priority + 1), right_power: priority };
}

// Look up the left and right binding power of a given infix or postfix operator.
export function bp_lookup(whichOperator: TokenType): binding_power {
    const no_binding_power: binding_power = {left_power: 0, right_power: 0};

    switch (whichOperator) {
        case TokenOperatorType.TOKEN_PLUS: return LeftAssociative(100);
        case TokenOperatorType.TOKEN_MINUS: return LeftAssociative(100);
        case TokenOperatorType.TOKEN_MULTIPLY: return LeftAssociative(200);
        case TokenOperatorType.TOKEN_DIVIDE: return LeftAssociative(200);
        case TokenOperatorType.TOKEN_POWER: return RightAssociative(99);
        case TokenOperatorType.TOKEN_QUESTION: return RightAssociative(1000);

        case TokenOperatorType.TOKEN_GREATER: return LeftAssociative(50);
        case TokenOperatorType.TOKEN_GREATER_EQUAL: return LeftAssociative(50);
        case TokenOperatorType.TOKEN_LESS: return LeftAssociative(50);
        case TokenOperatorType.TOKEN_LESS_EQUAL: return LeftAssociative(50);
        case TokenOperatorType.TOKEN_EQUAL: return LeftAssociative(50);
        case TokenOperatorType.TOKEN_NOT_EQUAL: return LeftAssociative(50);

        // --- Postfix --- (Always Right Associative)
        case TokenOperatorType.TOKEN_NOT: return RightAssociative(400);
        case TokenOperatorType.TOKEN_MEMBER: return RightAssociative(800);
        //Note: Postfix operators are always RightAssociative
        case TokenOperatorType.TOKEN_LEFT_PAREN: return RightAssociative(400);

        default: return no_binding_power;
    }
}

// Look up the right binding power of a given prefix operator
export function prefix_bp_lookup(whichOperator: TokenType): number {
    switch(whichOperator) {
        case TokenOperatorType.TOKEN_NOT: return 300;
        case TokenOperatorType.TOKEN_PLUS: return 300;
        case TokenOperatorType.TOKEN_MINUS: return 300;
        // case TokenOperatorType.TOKEN_LEFT_PAREN: return 900;
        default: return 0;
    }
};
