# Project 1: System Inspector

See: https://www.cs.usfca.edu/~mmalensek/cs326/assignments/project-2.html

TODO: Remove the link above. Your README should not depend on a link to the spec.

TODO: Replace this section with a short (1-3 paragraph) description of the project. What it does, how it does it, and any features that stand out. If you ever need to refer back to this project, the description should jog your memory.

## Building

TODO: Update this section as necessary.

To compile and run:

```bash
make
./program_name
```

## Program Options

TODO: Provide an overview of program options and their descriptions, if applicable. If the program does not accept any options, delete this section.

```bash
$ ./some_prog -h
Usage: ./some_prog [-z] [-d dir]

Options:
    * -d              Directory to load information from.
    * -z              Enable super secret 'Z' mode
```

## Included Files

* **file1.c** -- TODO: Describe any major program modules here.
* **file2.h** -- TODO: You don't need to include utilities, makefiles, etc.
* **file3.asm** -- TODO: Just the most important files! (And don't forget their descriptions)

## Testing

To execute the test cases, use `make test`. To pull in updated test cases, run `make testupdate`. You can also run a specific test case instead of all of them:

```
# Run all test cases:
make test

# Run a specific test case:
make test run=4

# Run a few specific test cases (4, 8, and 12 in this case):
make test run='4 8 12'

# Run a test case in gdb:
make test run=4 debug=on
```

If you are satisfied with the state of your program, you can also run the test cases on the grading machine. Check your changes into your project repository and then run:

```
make grade
```

## Demo Run

TODO: add a screenshot / text of a demo run of your program here.
