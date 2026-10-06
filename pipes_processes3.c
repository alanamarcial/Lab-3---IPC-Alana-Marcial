#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    // Validate command-line argument
    if (argc < 2)
    {
        fprintf(stderr, "Error: Missing search term argument.\nUsage: %s <search_term>\n", argv[0]);
        return 1;
    }

    int pipe1[2]; // Pipe connecting cat (stdout) -> grep (stdin)
    int pipe2[2]; // Pipe connecting grep (stdout) -> sort (stdin)

    // Create pipes
    if (pipe(pipe1) == -1)
    {
        perror("Error: Failed to create pipe 1");
        return 1;
    }

    if (pipe(pipe2) == -1)
    {
        perror("Error: Failed to create pipe 2");
        close(pipe1[0]);
        close(pipe1[1]);
        return 1;
    }

    // Fork P1: executes "cat scores"
    pid_t pid1 = fork();
    if (pid1 < 0)
    {
        perror("Error: Fork failed for process 1 (cat)");
        return 1;
    }

    if (pid1 == 0)
    {
        // Redirect stdout to pipe1 write end
        dup2(pipe1[1], STDOUT_FILENO);

        // Close pipe file descriptors cleanly
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);

        char *cat_args[] = {"cat", "scores", NULL};
        execvp("cat", cat_args);

        perror("Error: execvp failed for cat");
        exit(EXIT_FAILURE);
    }

    // Fork P2: executes "grep <arg>"
    pid_t pid2 = fork();
    if (pid2 < 0)
    {
        perror("Error: Fork failed for process 2 (grep)");
        return 1;
    }

    if (pid2 == 0)
    {
        // Redirect stdin from pipe1 read end and stdout to pipe2 write end
        dup2(pipe1[0], STDIN_FILENO);
        dup2(pipe2[1], STDOUT_FILENO);

        // Close pipe file descriptors cleanly
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);

        char *grep_args[] = {"grep", argv[1], NULL};
        execvp("grep", grep_args);

        perror("Error: execvp failed for grep");
        exit(EXIT_FAILURE);
    }

    // Fork P3: executes "sort"
    pid_t pid3 = fork();
    if (pid3 < 0)
    {
        perror("Error: Fork failed for process 3 (sort)");
        return 1;
    }

    if (pid3 == 0)
    {
        // Redirect stdin from pipe2 read end; stdout remains default stdout
        dup2(pipe2[0], STDIN_FILENO);

        // Close pipe file descriptors cleanly
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);

        char *sort_args[] = {"sort", NULL};
        execvp("sort", sort_args);

        perror("Error: execvp failed for sort");
        exit(EXIT_FAILURE);
    }

    // Parent process closes all pipe ends so EOF signals propagate properly
    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);

    // Parent explicitly waits for all child processes to prevent zombies
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);

    return 0;
}