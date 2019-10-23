---
title: vdbe
categories:
  - 技术文章
date: 2019-10-20 09:15:21
tags:
---

[The SQLite Bytecode Engine](https://www.sqlite.org/opcode.html)

通过EXPLAIN输出SQLite 字节码

# 指令格式
每个指令包含一个操作码和5个操作数P1、P2、P3、P4和P5.
操作数P1、P2、P3是32-bit signed integers，寄存器。 对于b-树指针的操作，P1用来存储cursor number。 
对于跳转指令，P2用来存储跳转的目的地址。 
P4可能是一个32-bit的signed integer、64-bit的signed integer、64-bit的floating point value、一个string literal、一个Blob literal、a pointer to a collating sequence comparison function、一个应用程序定义的SQL函数指针等。
P5是一个16-bit的unsigned integer用来存储flags。P5中flag的内容有时候会影响操作码的行为，if the SQLITE_NULLEQ (0x0080) bit of the P5 operand is set on the Eq opcode, then the NULL values compare equal to one another. Otherwise NULL values compare different from one another.

部分操作码会使用所有的五个操作数，有些操作码智慧使用其中的一两个，还有些操作码不使用操作数。

bytecode engine从号码为0的指令开始执行。Execution continues until a Halt instruction is seen, or until the program counter becomes greater than the address of last instruction, or until there is an error. When the bytecode engine halts, all memory that it allocated is released and all database cursors it may have had open are closed. If the execution stopped due to an error, any pending transactions are terminated and changes made to the database are rolled back.

# B树指针 (B-Tree Cursors）
A prepared statement can have zero or more open cursors. Each cursor is identified by a small integer, which is usually the P1 parameter to the opcode that uses the cursor. There can be multiple cursors open on the same index or table. All cursors operate independently, even cursors pointing to the same indices or tables. The only way for the virtual machine to interact with a database file is through a cursor. Instructions in the virtual machine can create a new cursor (ex: OpenRead or OpenWrite), read data from a cursor (Column), advance the cursor to the next entry in the table (ex: Next or Prev), and so forth. All cursors are automatically closed when the prepared statement is reset or finalized.

# Subroutines, Coroutines, and Subprograms
The bytecode engine has no stack on which to store the return address of a subroutine. Return addresses must be stored in registers. Hence, bytecode subroutines are not reentrant.

The Gosub opcode stores the current program counter into register P1 then jumps to address P2. The Return opcode jumps to address P1+1. Hence, every subroutine is associated with two integers: the address of the entry point in the subroutine and the register number that is used to hold the return address.

The Yield opcode swaps the value of the program counter with the integer value in register P1. This opcode is used to implement coroutines. Coroutines are often used to implement subqueries from which content is pulled on an as-needed basis.

Triggers need to be reentrant. Since bytecode subroutines are not reentrant a different mechanism must be used to implement triggers. Each trigger is implemented using a separate bytecode program with its own opcodes, program counter, and register set. The Program opcode invokes the trigger subprogram. The Program instruction allocates and initializes a fresh register set for each invocation of the subprogram, so subprograms can be reentrant and recursive. The Param opcode is used by subprograms to access content in registers of the calling bytecode program.

# Self-Altering Code
Some opcodes are self-altering. For example, the Init opcode (which is always the first opcode run a bytecode program) increments its P1 operand. Subsequent Once opcodes compare their P1 operands to the P1 value for the Init opcode in order to determine if the one-time initialization code that follows should be skipped. Another example is the String8 opcode which converts its P4 operand from UTF-8 into the correct database string encoding, then converts itself into a String opcode.

# 查看字节码
每一个SQL语句都会被转换成字节码执行。可以通过在SQLite语句前加上EXPLAIN打印出字节码，而不去执行。如下：
```sql
root@batman:~# sqlite3 test.db
SQLite version 3.11.0 2016-02-15 17:29:24
Enter ".help" for usage hints.
sqlite> .table
info
sqlite> EXPLAIN SELECT * FROM info;
addr  opcode         p1    p2    p3    p4             p5  comment
----  -------------  ----  ----  ----  -------------  --  -------------
0     Init           0     10    0                    00  Start at 10
1     OpenRead       0     2     0     3              00  root=2 iDb=0; info
2     Rewind         0     8     0                    00
3       Column         0     0     1                    00  r[1]=info.name
4       Column         0     1     2                    00  r[2]=info.age
5       Column         0     2     3                    00  r[3]=info.salary
6       ResultRow      1     3     0                    00  output=r[1..3]
7     Next           0     3     0                    01
8     Close          0     0     0                    00
9     Halt           0     0     0                    00
10    Transaction    0     0     1     0              01  usesStmtJournal=0
11    TableLock      0     2     0     info           00  iDb=0 root=2 write=0
12    Goto           0     1     0                    00
sqlite>
```

# 操作码
目前虚拟机定义了170个操作码：
1. Abortable
Verify that an Abort can happen. Assert if an Abort at this point might cause database corruption. This opcode only appears in debugging builds.
An Abort is safe if either there have been no writes, or if there is an active statement journal.

2. Add
Add the value in register P1 to the value in register P2 and store the result in register P3. If either input is NULL, the result is NULL.

3. AddImm
Add the constant P2 to the value in register P1. The result is always an integer.
To force any register to be an integer, just add 0.

4. Affinity
Apply affinities to a range of P2 registers starting with P1.
P4 is a string that is P2 characters long. The N-th character of the string indicates the column affinity that should be used for the N-th memory cell in the range.

5. AggFinal
P1 is the memory location that is the accumulator for an aggregate or window function. Execute the finalizer function for an aggregate and store the result in P1.
P2 is the number of arguments that the step function takes and P4 is a pointer to the FuncDef for this function. The P2 argument is not used by this opcode. It is only there to disambiguate functions that can take varying numbers of arguments. The P4 argument is only needed for the case where the step function was not previously called.

6. AggInverse
Execute the xInverse function for an aggregate. The function has P5 arguments. P4 is a pointer to the FuncDef structure that specifies the function. Register P3 is the accumulator.
The P5 arguments are taken from register P2 and its successors.

对聚集执行xInverse函数。该函数具有P5参数。 P4是指向指定函数的FuncDef结构的指针。寄存器P3是累加器。
P5自变量取自寄存器P2及其后续寄存器。

7. AggStep
Execute the xStep function for an aggregate. The function has P5 arguments. P4 is a pointer to the FuncDef structure that specifies the function. Register P3 is the accumulator.
The P5 arguments are taken from register P2 and its successors.


