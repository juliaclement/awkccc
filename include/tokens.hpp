/***
**
** AWKCCC: Interpreter tokens
**
** Copyright (C) 2022 Julia Ingleby Clement
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
***/
namespace awkcc {
    enum addressing {
        // High order 3 bits of op code [0..7]
        STACK_RELATIVE = 0,
        VARIABLE_STORE,
        LOCAL_STORE, // Current function local variables
        ARRAY_INDEX, // TOS = Array, 2ndOS = Index in Array
        FUNCTIONS,
        BUILT_IN,
        IMMEDIATE,        // low order 23 bits of op code as unsigned. Range = 0..8,388,607
        GUARD3 = IMMEDIATE
    };
    enum opcodes {
        // First 32 (Through GUARD5) shared as lexer tokens
        // First 64 (Through GUARD6) shared as low order 6 bits of opcode in interpreter
        LOAD_NUM=0,         // move operand to top of stack as number
        LOAD_STR,           // move operand to top of stack as string
        LOAD_REGEX,         // move operand to top of stack as regex
        LOADNEGATIVE,       // move negated numeric operand to top of stack
        STORE,              // = (Assign operator for results of computation)
        POP,                // remove N elements from stack. 0 = discard one, >=1 = convert to std::Vector and leave pointer on top of stack
        ADD,
        SUB,
        MUL,
        DIV,
        MODULO,
        INCR,               // ++
        DECR,               // --
        XOR,
        OR,
        AND,
        CONCAT,             // string concatenate
        // Compare TOS against value, set TRUE/FALSE flag.
        LE,
        LT,
        GT,
        GE,
        EQ,
        NE,                 // = NOT EQ
        MATCH,              // regex match (sets TRUE/FALSE flag)
        NOTMATCH,           // regex match fails (sets TRUE/FALSE flag)
        // The following 3 ops don't use memory other than TOS & TRUE/FALSE flag
        RETURN,             // Return a value
        NEGATE,
        NOT,
        // End of dual purpose tokens
        GUARD5 = NOT,
        CALL,
        CALL_CODE,          // Call code procedure (e.g. cos, substr, etc)
        BRANCH,
        BRANCH_TRUE,
        BRANCH_FALSE,
        // The following ops don't use memory other than TOS & TRUE/FALSE flag
        ENTER,              // Enter a procedure
        DUP,                // Duplicate TOS
        PUSH_TEST_RESULT,   // True -> 1, False -> 0
        SWAP,               // exchange TOS & 2nd on stack
        GUARD6              // Tested to make sure that all interpreter ops fit in 6 bits
    };
    enum tokens {
        // Non-interpreter
        LBRACE='{',
        RBRACE='}',
        LBRACKET='[',
        RBRACKET=']',
        LPAREN='(',
        RPAREN=')',
    };
    static_assert(int(addressing::GUARD3) < 8);
    static_assert(int(opcodes::GUARD5) < 32);
    static_assert(int(opcodes::GUARD6) < 64);
    struct instruction {
        addressing memory_mode:3;
        opcodes op_code:6;
        unsigned operand:23;
    };
    static_assert( sizeof( struct instruction) == 4);
};
