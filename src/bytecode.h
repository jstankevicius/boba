#pragma once

enum class Instruction : unsigned char {
    PushInt = 1,
    PushStr,
    PushFloat,
    PushRef,
    PushTrue,
    PushFalse,
    PushNil,

    STORE,

    Add,
    Sub,
    Mul,
    Div,

    Neg,
};
