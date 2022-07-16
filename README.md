# Stutter

Stutter (name to be finalized later) is a simple Lisp dialect that runs on a stack machine with a custom bytecode instruction set. Stutter's bytecode is quite compact, although this is typical for a stack machine - most instructions are 1 or 5 bytes long, depending on whether or not they take an integer as an argument. It's possible to shrink the upper bound even further, but that would require a lot of time and effort which could be spent on developing other language features.

The eventual goal of this project is to become a general-purpose Lisp dialect that can do most things that other programming languages can.

## Building
Building and running Stutter is really simple.

Building the binary:
```
make
```

Running a Stutter program (the extension doesn't actually matter, the file just has to be encoded in ASCII):
```
build/stutter file.stt
```

Running tests:
```
make test
```

The repository ships with the single-include Catch2 header, so no external dependencies are required for test running.
