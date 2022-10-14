# Unix Shell

I have attempted to write a shell program purely in C and have done the following functionalities:

- Some Internal commands
    - cd
    - exit
    - help
    - jobs
- I/O redirection using `<>` operator.
- Functionality of the pipe operator `|`
- Multiple commands on a single line separated by semi colons `;`
- Command History
- IF Control structure (incomplete)
- Environment Variables
- Auto file name complete using `tab` key

## Source code Tree

```
.
├── .history.hist
├── Makefile
├── READMME.md
├── include
│   ├── builtin.h
│   └── command.h
└── src
    ├── builtin.c
    ├── command.c
    └── main.c

2 directories, 8 files
```

## How to run?

Simple download the source code and open terminal in he myshell directory. There is a Makefile in the source folder so just need to run the following command:

```bash
make
```

It will create a new directory bin and place your executable there and you can run you shell by:

```bash
./bin/myshell
```
