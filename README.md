# Mini-Shell

Mini-Shell is a simple shell program written in C that mimics basic Unix shell commands. This project demonstrates basic concepts of process management, file handling, and command execution in a Unix-like environment.

## Features

- Execute common Unix commands such as `ls`, `pwd`, `touch`, `cat`, `rm`, `echo`, `mkdir`, `mv`, `cal`, `who`, `more`, `sort`, `uniq`, `ps`, `kill`, `cmp`, and `history`.
- Implement command history functionality.
- Execute commands in pipelines.
- Display information about running processes.

## Requirements

- A Unix-like operating system (Linux, macOS, etc.)
- GCC (GNU Compiler Collection) or any other C compiler

## Installation

1. Clone the repository:

    ```bash
    git clone https://github.com/your-username/mini-shell.git
    cd mini-shell
    ```

2. Compile the source code:

    ```bash
    gcc -o mini-shell mini-shell.c
    ```

## Usage

To start the mini-shell, simply run the compiled executable:

```bash
./mini-shell
```

You will be presented with a prompt where you can enter various commands.

### Available Commands

- `exit`: Exit the mini-shell.
- `pwd`: Print the current working directory.
- `touch <filename>`: Create an empty file.
- `cat <filename>`: Display the contents of a file.
- `rm <filename>`: Remove a file.
- `echo <text>`: Print text to the terminal.
- `ls [directory]`: List files in the current or specified directory.
- `mkdir <directory>`: Create a new directory.
- `mv <source> <destination>`: Move or rename a file or directory.
- `cal [month] [year]`: Display a calendar.
- `who`: Display who is logged on.
- `more <filename>`: Display file contents with pagination.
- `sort <filename>`: Sort the contents of a file.
- `uniq <filename>`: Remove duplicate lines from a file.
- `ps`: List running processes.
- `kill <pid>`: Terminate a process by its PID.
- `cmp <file1> <file2>`: Compare two files byte by byte.
- `history`: Display the command history.

### Example

```sh
mini-shell$ pwd
Current directory: /home/user/mini-shell

mini-shell$ touch test.txt

mini-shell$ echo "Hello, Mini-Shell!" > test.txt

mini-shell$ cat test.txt
Hello, Mini-Shell!

mini-shell$ history
Command history:
1. pwd
2. touch test.txt
3. echo "Hello, Mini-Shell!" > test.txt
4. cat test.txt
5. history
```

## Implementation Details

### Command Parsing

Commands entered in the shell are parsed using the `parse_command` function, which tokenizes the input string into individual arguments.

### Command Execution

Commands are executed using the `execute_command` function, which forks a new process and uses `execvp` to run the specified command.

### Pipeline Execution

Pipelines are handled by the `execute_pipeline` function, which sets up a pipe and forks two processes to handle the input and output of the commands.

### Process Listing

The `list_processes` function lists all running processes by reading from the `/proc` directory and extracting relevant information from the `/proc/[pid]/stat` files.

