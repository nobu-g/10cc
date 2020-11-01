# 10cc: a simple C compiler

This project is a reimplementation of [rui314's 9cc](https://github.com/rui314/9cc)

- Supported ISA
  - x86_64
- Assembly Language
  - Intel syntax

## Features
- operations
  - binary: +, -, *, /, ==, !=, <=, >=, <, >, =
  - unary: +, -, &, *, sizeof, ++, --
- statements
  - if, for, while, return
- types
  - int, char, pointer, array
- local/global variable
- function
- variable scope
- string literal
- comments

## Reference
[低レイヤを知りたい人のためのCコンパイラ作成入門](https://www.sigbus.info/compilerbook)
