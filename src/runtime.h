/*


  Instruction format:
  1 byte per instruction type. This means a total of 256 possible instructions.

  Types:

  push_i          Push an integer onto the stack
  push_str        Push a string onto the stack
  push_f          Push a float onto the stack
  push_ref        Get a symbol's value and push it onto the stack

  push_true       Push the boolean "true" onto the stack
  push_false      Push the boolean "false" onto the stack
  push_nil        Push "nil" onto the stack

  store           Store whatever variable is on the stack into the current environment

  add_i           Add two integers
  add_f           Add two floats

  sub_i           Subtract two integers
  sub_f           Subtract two floats
  neg_i           Negate an integer
  neg_f           Negate a float

  mul_i           Multiply two integers
  mul_f           Multiply two floats

  div_i           Divide two integers
  div_f           Divide two floats

  Variables!

  How do we number off variables?

  Can try doing it the same way as with strings, but need more context to decide
  if a variable has a different meaning. Each variable index is *for the current
  environment*, so we don't need to worry about what happens during recursion.

  For example:

  (def a 1) ; <= 'a' gets assigned index 0
  (defun f () ; <= 'f' gets assigned index 1
    (def a 2) ; <= This is a different scope now, so this 'a' gets index 2


  Any time we define a variable we should check the current scope.
  If the variable exists in the current scope:
    error - variable redefinition
  Else:
    Increment counter and add (counter: symbol) to variable_entries

*/
#pragma once

#include <any>
#include <cstdint>
#include <deque>
#include <memory>
#include <unordered_map>
#include <vector>

#include "ast.h"
#include "environment.h"
#include "processor.h"


class Runtime {

private:
    Processor proc;

    void emit_push_int(int i);
    void emit_push(std::shared_ptr<AST> ast);
    void emit_def(std::shared_ptr<AST> ast);
    void emit_expr(std::shared_ptr<AST> ast);

public:

    Runtime() {
        proc.envs.push_back(Environment());
        std::memset(proc.instructions, 0, 1024);
    }

    void eval_ast(std::shared_ptr<AST> ast);
};
