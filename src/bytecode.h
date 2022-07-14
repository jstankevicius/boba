

enum class Instruction : unsigned char {
    PUSH_I = 1,
    PUSH_STR,
    PUSH_F,
    PUSH_REF,
    PUSH_TRUE,
    PUSH_FALSE,
    PUSH_NIL,

    STORE,

    ADD_I,
    ADD_F,

    SUB_I,
    SUB_F,

    NEG_I,
    NEG_F,

    MUL_I,
    MUL_F,

    DIV_I,
    DIV_F
};
