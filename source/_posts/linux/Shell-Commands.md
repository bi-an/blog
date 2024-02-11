---
title: Shell Commands
date: 2024-01-26 15:57:35
categories: Linux
tags: shell
---


## shuf

## cut

## tr

## lp

## sort

Options:

    -t, --field-separator=SEP
        use SEP instead of non-blank to blank transition

    -k, --key=POS1[,POS2]
        start a key at POS1 (origin 1), end it at POS2 (default end of line)

    -h, --human-numeric-sort
        compare human readable numbers (e.g., 2K 1G)

    -n, --numeric-sort
        compare according to string numerical value

## nproc

print the number of processing units avaiable.

## od / xxd / hexdump

read the binary file.

Notes: byte order

```bash
$ echo -n "ABCD" | xxd
00000000: 4142 4344                                ABCD
$ echo -n "ABCD" | hexdump
0000000 4241 4443                              
0000004
```

[Reference](https://unix.stackexchange.com/questions/282215/how-to-view-a-binary-file)

## comm / diff / tkdiff / cmp

Can be used to compare binary or non-binary files.

### comm

compare two sorted files line by line.

```bash
$ cat file1.txt 
apple
banana
cherry

$ cat file2.txt 
banana
cherry
date
erase

$ comm file1.txt file2.txt 
apple
                banana
                cherry
        date
        erase
```

The file must be sorted before using the `comm` command. Otherwise it will complain that:

    comm: file 1 is not in sorted order

and cannot work correctly. For example,

```bash
$ cat file1.txt 
apple
cherry
banana

$ cat file2.txt 
banana
cherry
date
erase

$ comm file1.txt file2.txt 
apple
        banana
                cherry
comm: file 1 is not in sorted order
banana
        date
        erase
comm: input is not in sorted order
```

### diff

Syntax:

    diff -u file1 file2

Options:

    -e, --ed
        output an ed script

    -u, -U NUM, --unified[=NUM]
        output NUM (default 3) lines of unified context
        (that is, print NUM lines before and after the difference line)

### tkdiff

Use a GUI to display the differences.

### cmp

Prints less information comparing to `diff`.

Syntax:

    cmp file1 file2


## ed/vim/sed/awk
