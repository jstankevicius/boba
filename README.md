# 🧋Boba
[![boba-tests](https://github.com/jstankevicius/boba/workflows/boba-tests/badge.svg)](https://github.com/jstankevicius/boba/actions)

Boba is a simple Lisp dialect that runs on a stack machine with a custom bytecode instruction set. Boba's bytecode is quite compact, although this is typical for a stack machine - all instructions are either 1 or 5 bytes long, depending on whether or not they take an integer as an argument.

The eventual goal of this project is to become a general-purpose Lisp dialect that can do most things that other programming languages can.

## Building and running the interpreter:
Building and running Boba is really simple.

Building the binary:
```
make
```

Running a Boba program (the extension doesn't actually matter, the file just has to be encoded in ASCII):

### fib.boba
```
; Find the n-th fibonacci number
(def fib (fn (n)

      ; If n == 0, return 0
      (if (= n 0)
          0

          ; Else, if n == 1, return 1
          (if (= n 1)
              1

              ; Else, if n == 2, return 1
              (if (= n 2)
                 1

                 ; If not any of those, return fib(n - 1) + fib(n - 2)
                 (+ (fib (- n 1)) (fib (- n 2))))))))

(fib 9)
```

Run the binary:
```
$ build/boba fib.boba
34
```

## Running tests:
```
make test
```

The repository ships with the single-include Catch2 header, so no external dependencies are required for test running.
