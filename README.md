# toypiler

This project is a compiler of a toy language to machine code for a virtual
machine. In the [first section](#virtual-machine) we describe the machine code
for virtual machine and how it works, in the
[second section](#compiled-language) we describe the language's grammar.

Code for compiling the language and for interpreting compiled machine code is in
[](compiler.c) and [](interpreter.c) files respectively.

The compiler's output is a text recording of the code for the virtual machine.
Each instruction is on a separate line, if an instruction has arguments they are
separated by spaces. There should be one `HALT` command at the end of a machine
code. The interpreter assumes the correctness of the machine code. If an error
is detected, it reports an assertion violation. The compiler assumes the
correctness of the language. If an error is detected it prints an error message
to stderr and exits with an error exit code.

## Virtual machine
Virtual machine code instructions have non-negative integer addresses. VM's data
is stored on stacks with pop and push operations. VM has stacks of bits,
numbered with non-negative integers. VM also has a stack of instructions'
addresses, which we're going to call the return stack.

The machine recognizes ten types of instructions, with codes from 0 to 9:
0. `PUSH_0 stack` push bit 0 on stack with the number `stack`,
1. `PUSH_1 stack` push bit 1 on stack with the number `stack`,
2. `OUTPUT_0` Output bit 0,
3. `OUTPUT_1` Output bit 1,
4. `POP_BRANCH address stack` Pop a bit from the stack with the number `stack`, if the bit has value 0 then go to the instruction at the address of `address`, otherwise go to the next instruction,
5. `INPUT_BRANCH address` Read a bit from the input, if the bit has value 0 then go to the instruction at the address of `address`, otherwise go to the next instruction,
6. `JUMP address` go to the instruction at the address of `address`,
7. `CALL address` push the address of the next instruction after this `CALL` instruction, on the return stack and go to the instruction at the address of `address`,
8. `RETURN` Pop an address from the return stack and go to the instruction at that address,
9. `HALT` Halt the machine.

Instruction `INPUT_BRANCH` reads bits of input characters in order of the most
significant bits. When the end of data is reached, it reads eight consecutive
bits of 1. Instructions `OUTPUT_0` and `OUTPUT_1` write bits of characters to
output in order from the most significant bits.

The execution of the program begins with the instruction at the address 0.
After an instruction is executed, the instruction located next in the code is
executed, unless the instruction specified otherwise. If the instruction the
machine were to go to does not exist, then the code has an error.

## Compiled language
The syntax of the language is described by a grammar with an initial symbol
`Program`:

```
Program → ProcedureSequence MainProcedure
MainProcedure → Body
ProcedureSequence → ε | Procedure ProcedureSequence
Procedure → Name Body
Name → A | B | C | D | E | F | G | H | I | J | K | L | M | N | O | P | Q | R | S | T | U | V | W | X | Y | Z
Body → { StatementSequence }
StatementSequence → ε | Statement StatementSequence
Statement → Call | Write | Choice
Call → Name
Write → Destination BitSequence
Destination → StackName | Special
StackName → a | b | c | d | e | f | g | h | i | j | k | l | m | n | o | p | q | r | s | t | u | v | w | x | y | z
Special → $
BitSequence → Bit BitSequence | Bit
Bit → - | +
Choice → Source Body Source
Source → StackName | Special
```

In the program, in addition to the characters representing the end symbols of
the grammar, the following separators can occur anywhere:
- spaces, tabs, endlines,
- comments from `;` to the end of the line.
They do not affect the interpretation of a program.

A program consists of a string of named procedures and an anonymous main
procedure, analogous to the `main()` function in C. A procedure's name is a
capital letter, a stack's name is a lowercase letter. The execution of a program
begins with the main procedure. The content of a procedure is a string of
statements.

There are three types of statements:
1. Call - Executes a procedure. After it is finished, the program return to the address of call.
2. Write - Pushes a non-empty string of bits on a stack with the given name. If the stack's
name is special (`$`) then writes the string to output. Bits of 0 and 1 are
represented by characters `-` and `+` respectively. The bits are written to a
stack or to output in order of occurrence in the sequence from left to right.
3. Choice - Performs one of two statements depending on the value of the bit. If the name of
the stack is given, chooses based on the value of the bit popped from that
stack. If instead of the name of the stack a special name is given (`$`), make
the choice based on a bit loaded from the input. If the bit has a value 1, the
first of the given statements is executed, otherwise the second statement is
executed.
