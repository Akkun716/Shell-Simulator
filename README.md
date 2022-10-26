# Project 2: Shell Simulator

This is a shell implemented using the C language. This shell named `fish` (Frequently Inconsistant SHell), provides functionality similar to the `bash` shell. Typical shell operations: prompt user input, executes built-in and command functions that are available in the current directory and PATH environment variable, scripting. 
## Building

To compile and run:

```bash
make
./fish
```

## Program Options

```bash
$ ./some_prog
Usage: ./some_prog

## Included Files

* **shell.c** -- This is the primary runner for the project. It contains the runner function for the project and the shell capabilities for the simulator. 
* **history.c** -- The history files provides the functions for managing and maintaining the history structure. Functionality like addition, removal, searching capabilities (based on prefix or command number), and printing out the contents of the history structure.
* **history.h**
* **linkedhistory.c** -- The linkedhistory files are the foundation of the history structure and background job list. These provide the fundamental linked list abilities needed for those structures, along with some other capabilities. One thing to be noted is the `append_node` function, as it has the id parameter. This is what allows this LinkedHistory structure to be used for both the history and the background list. -1 is passed to enable default id assignment, while any positive value sets that entry\'s id to the passed value.
* **linkedhistory.h**
* **ui.c** -- The ui files provide the overall visual element to the project, along with special keyboard input. When the command `./fish` is run, a prompt is displayed, which simulates a shell terminal prompt, including current location within the device\'s registries and the current user of the device. Regarding keyboard input, the user can press the up and down arrows to navigate through the command history as one would in any other terminal shell, as well as being able to use the tab key to autocomplete a command.
* **ui.h**

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
