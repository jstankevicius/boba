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

```
$ build/boba yourfile.boba
```


## Feature Examples
**Recursion!** As any normal programming language should, Boba supports recursion. Recursive calls are currently not tail-call optimized, although that is a feature that will be implemented at some point in the future.

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

```
$ build/boba fib.boba
34
```

**Closures and first-class function support!** A core feature of many Lisp languages, functions being first-class objects means they can be returned from and passed to functions. Functions (or rather, closures) are not simply instruction routines - they are actual runtime objects with an entire enclosing environment. This allows for the creation of, for example, functions that return other functions.

### closure.boba
```
(def addto (fn (x) (fn (y) (+ x y))))
(def plus4 (addto 4))
(plus4 1)
```

```
$ build/boba closure.boba
5
```

## Running tests:
```
make test
```

Boba tests simply compile and run expressions while checking them against the expected result. The repository used to ship with the single-include Catch2 header, but I eventually decided that an 18,000-line header file is overkill for the kind of testing that we'll be doing.
