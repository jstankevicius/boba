enum class Instruction : unsigned char
{
    // Pushing stuff onto the stack:
    PushInt = 1,
    PushStr,
    PushFloat,
    PushRef,
    
    PushTrue,
    PushFalse,
    
    PushNil,

    // Store:
    Store,

    // Relative jumps:
    Jmp,
    JmpTrue,
    JmpFalse,

    // Call directly into the closure without putting it on the stack.
    Call,

    // Call a closure from the top of the stack, popping it
    CallPop,
    CreateClosure,
    
    Ret,

    // Logic:
    Not,
    And,
    Or,

    // Comparison:
    Eq,
    Greater,
    GreaterEq,
    Less,
    LessEq,

    // Arithmetic:
    Add,
    Sub,
    Mul,
    Div,
    Neg,
};
