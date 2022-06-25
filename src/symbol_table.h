#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

#include "symbol.h"

class SymbolTable {

private:
    struct Scope {
        std::unordered_map<std::string, Symbol> symbols;
    };

    std::vector<std::shared_ptr<Scope>> scopes;

public:
    inline void enter_scope() {
        scopes.push_back(std::make_shared<Scope>());
    }

    inline void exit_scope() {
        scopes.pop_back();
    }

    inline int num_scopes() {
        return scopes.size();
    }

    inline bool symbol_exists(std::string name) {
        return true;
    }

    inline void insert_symbol(std::string name, Symbol sym) {
        auto cur_scope = scopes.back();
        cur_scope.get()->symbols.emplace(name, sym);
    }
};

/*

instructions:
PUSH $value
ADD $num_args
SUB $num_args
MUL $num_args
DIV $num_args

; (def x (+ 1 3))
PUSH 1
PUSH 3
ADD 2
BIND 1

(defun f (n)
    (def x n)
    (if (= n 0)
        (print "n is 0")
    (f (- n 1))))

f:

// before a function is called, its arguments must be on the stack
BIND n
PUSH n
BIND x
EQ n 0
JMP 5
PUSH n
PUSH 1
SUB
PUSH x // save old value of "x"
CALL f // push ip + 1 onto stack
BIND x // restore value of local variable "x"
PUSH "n is 0"
CALL print
RETURN // jump to return address on stack
*/

