#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_COMMAND_LENGTH 100
#define MAX_ARGS 10
#define MAX_HISTORY_SIZE 20

typedef struct {
    char command[MAX_COMMAND_LENGTH];
} Command;


void print_process_info(const char* pid) {
    char proc_path[256];
    snprintf(proc_path, sizeof(proc_path), "/proc/%s/stat", pid);

    FILE* file = fopen(proc_path, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open %s\n", proc_path);
        return;
    }

    // Read process information from /proc/[pid]/stat file
    char comm[256];
    char state;
    int ppid;
    int priority;
    fscanf(file, "%*d %s %c %d %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %d", comm, &state, &ppid, &priority);
    fclose(file);

    printf("%-10s %-6s %-6d %-6d\n", pid, comm, ppid, priority);
}

void list_processes() {
    DIR* dir = opendir("/proc");
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Check if the entry is a directory and represents a process ID
        if (entry->d_type == DT_DIR && atoi(entry->d_name) != 0) {
            print_process_info(entry->d_name);
        }
    }

    closedir(dir);
}

void execute_command(char** args) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    } else if (pid == 0) {
        // Child process
        execvp(args[0], args);
        // If execvp returns, an error occurred
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        wait(NULL);
    }
}

int parse_command(char* command, char** args) {
    int argc = 0;
    char* token = strtok(command, " \t\n");

    while (token != NULL) {
        args[argc] = token;
        argc++;
        token = strtok(NULL, " \t\n");
    }

    args[argc] = NULL;
    return argc;
}

int execute_pipeline(char** args1, char** args2) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }

    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("fork");
        return -1;
    } else if (pid1 == 0) {
        // Child process 1
        close(pipefd[0]); // Close unused read end
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
        close(pipefd[1]); // Close write end

        execvp(args1[0], args1);
        perror("execvp");
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        return -1;
    } else if (pid2 == 0) {
        // Child process 2
        close(pipefd[1]); // Close unused write end
        dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to pipe
        close(pipefd[0]); // Close read end

        execvp(args2[0], args2);
        perror("execvp");
        exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    wait(NULL);
    wait(NULL);

    return 0;
}

void display_file(char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("more");
        return;
    }

    char line[1024];
    int line_count = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);
        line_count++;

        if (line_count % 10 == 0) {
            printf("--More--");
            fflush(stdout);

            char c;
            while ((c = getchar()) != EOF) {
                if (c == 'q') {
                    printf("\n");
                    fclose(file);
                    return;
                } else if (c == '\n') {
                    break;
                }
            }
        }
    }

    fclose(file);
}

void sort_file(char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("sort");
        return;
    }

    char lines[1024][1024];
    int line_count = 0;

    while (fgets(lines[line_count], sizeof(lines[line_count]), file) != NULL) {
        line_count++;
    }

    for (int i = 0; i < line_count - 1; i++) {
        for (int j = i + 1; j < line_count; j++) {
            if (strcmp(lines[i], lines[j]) > 0) {
                char temp[1024];
                strcpy(temp, lines[i]);
                strcpy(lines[i], lines[j]);
                strcpy(lines[j], temp);
            }
        }
    }

    for (int i = 0; i < line_count; i++) {
        printf("%s", lines[i]);
    }

    fclose(file);
}

void uniq_file(char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("uniq");
        return;
    }

    char prev_line[1024] = "";
    char line[1024];

    while (fgets(line, sizeof(line), file) != NULL) {
        if (strcmp(line, prev_line) != 0) {
            printf("%s", line);
            strcpy(prev_line, line);
        }
    }

    fclose(file);
}
void compare_files(char* filename1, char* filename2) {
    FILE* file1 = fopen(filename1, "rb");
    FILE* file2 = fopen(filename2, "rb");

    if (file1 == NULL) {
        perror("cmp");
        return;
    }

    if (file2 == NULL) {
        perror("cmp");
        fclose(file1);
        return;
    }

    int byte1, byte2;
    int pos = 0;
    int diff = 0;

    while (1) {
        byte1 = fgetc(file1);
        byte2 = fgetc(file2);
        pos++;

        if (byte1 == EOF || byte2 == EOF) {
            if (byte1 != byte2) {
                diff = 1;
            }
            break;
        }

        if (byte1 != byte2) {
            diff = 1;
            break;
        }
    }

    if (diff) {
        printf("%s %s differ: byte %d\n", filename1, filename2, pos);
    } else {
        printf("Files %s and %s are identical.\n", filename1, filename2);
    }

    fclose(file1);
    fclose(file2);
}

int main() {
    char command[MAX_COMMAND_LENGTH];
    char* args[MAX_ARGS];
    Command history[MAX_HISTORY_SIZE];
    int history_count = 0;

    while (1) {
        printf("mini-shell$ ");
        fflush(stdout);

        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }
	// Store command in history
        Command new_command;
        strncpy(new_command.command, command, sizeof(new_command.command));
        if (history_count < MAX_HISTORY_SIZE) {
            history[history_count] = new_command;
            history_count++;
        } else {
            // Shift history elements to make space for the new command
            for (int i = 0; i < MAX_HISTORY_SIZE - 1; i++) {
                history[i] = history[i + 1];
            }
            history[MAX_HISTORY_SIZE - 1] = new_command;
        }
        int argc = parse_command(command, args);

        if (argc > 0) {
            if (strcmp(args[0], "exit") == 0) {
                break;
            } else if (strcmp(args[0], "pwd") == 0) {
                char cwd[1024];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("Current directory: %s\n", cwd);
                } else {
                    perror("getcwd");
                }
            } else if (strcmp(args[0], "touch") == 0) {
                if (argc >= 2) {
                    FILE* file = fopen(args[1], "w");
                    if (file != NULL) {
                        fclose(file);
                    } else {
                        perror("touch");
                    }
                } else {
                    printf("Usage: touch <filename>\n");
                }
            } else if (strcmp(args[0], "cat") == 0) {
                if (argc >= 2) {
                    if (strcmp(args[1], "-") == 0) {
                        // Read from standard input
                        char line[1024];
                        while (fgets(line, sizeof(line), stdin) != NULL) {
                            printf("%s", line);
                        }
                    } else {
                        FILE* file = fopen(args[1], "r");
                        if (file != NULL) {
                            char ch;
                            while ((ch = fgetc(file)) != EOF) {
                                putchar(ch);
                            }
                            fclose(file);
                        } else {
                            perror("cat");
                        }
                    }
                } else {
                    printf("Usage: cat <filename>\n");
                }
            } else if (strcmp(args[0], "rm") == 0) {
                if (argc >= 2) {
                    if (remove(args[1]) == -1) {
                        perror("rm");
                    }
                } else {
                    printf("Usage: rm <filename>\n");
                }
            } else if (strcmp(args[0], "echo") == 0) {
                for (int i = 1; i < argc; i++) {
                    printf("%s ", args[i]);
                }
                printf("\n");
            } else if (strcmp(args[0], "ls") == 0) {
                DIR* dir;
                struct dirent* entry;

                if (argc == 1) {
                    dir = opendir(".");
                } else if (argc == 2) {
                    dir = opendir(args[1]);
                } else {
                    printf("Usage: ls [directory]\n");
                    continue;
                }

                if (dir == NULL) {
                    perror("ls");
                } else {
                    while ((entry = readdir(dir)) != NULL) {
                        printf("%s\n", entry->d_name);
                    }
                    closedir(dir);
                }
            } else if (strcmp(args[0], "mkdir") == 0) {
                if (argc >= 2) {
                    if (mkdir(args[1], 0777) == -1) {
                        perror("mkdir");
                    }
                } else {
                    printf("Usage: mkdir <directory>\n");
                }
            } else if (strcmp(args[0], "mv") == 0) {
                if (argc >= 3) {
                    if (rename(args[1], args[2]) == -1) {
                        perror("mv");
                    }
                } else {
                    printf("Usage: mv <source> <destination>\n");
                }
            } else if (strcmp(args[0], "cal") == 0) {
                if (argc == 1) {
                    system("cal");
                } else if (argc == 2) {
                    char cal_command[MAX_COMMAND_LENGTH];
                    snprintf(cal_command, sizeof(cal_command), "cal %s", args[1]);
                    system(cal_command);
                } else {
                    printf("Usage: cal [month] [year]\n");
                }
            } else if (strcmp(args[0], "who") == 0) {
                if (argc == 1) {
                    system("who");
                } else {
                    printf("Usage: who\n");
                }
            } else if (strcmp(args[0], "more") == 0) {
                if (argc >= 2) {
                    display_file(args[1]);
                } else {
                    printf("Usage: more <filename>\n");
                }
            } else if (strcmp(args[0], "sort") == 0) {
                if (argc >= 2) {
                    sort_file(args[1]);
                } else {
                    printf("Usage: sort <filename>\n");
                }
            } else if (strcmp(args[0], "uniq") == 0) {
                if (argc >= 2) {
                    uniq_file(args[1]);
                } else {
                    printf("Usage: uniq <filename>\n");
                }
            } else if (strcmp(args[0], "ps") == 0) {
            	    printf("%-10s %-6s %-6s %-6s\n", "PID", "CMD", "PPID", "PRIORITY");
    		    list_processes();
            } else if (strcmp(args[0], "kill") == 0) {
                if (argc == 2) {
                    pid_t pid = atoi(args[1]);
                    if (kill(pid, SIGTERM) == 0) {
                        printf("Process with PID %d killed.\n", pid);
                    } else {
                        perror("kill");
                    }
                } else {
                    printf("Usage: kill <pid>\n");
                }
            } else if (strcmp(args[0], "cmp") == 0) {
                if (argc == 3) {
                    compare_files(args[1], args[2]);
                } else {
                    printf("Usage: cmp <file1> <file2>\n");
                }
            } else if (strcmp(args[0], "history") == 0) {
                printf("Command history:\n");
                for (int i = 0; i < history_count; i++) {
                    printf("%d. %s", i + 1, history[i].command);
                }
            } else {
                printf("Command not found: %s\n", args[0]);
            }
        }
    }

    return 0;
}
