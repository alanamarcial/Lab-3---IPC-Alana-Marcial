/**
 * @file pipes_processes1.c
 * @author Alana Marcial
 * @date October 5, 2026
 * @course Operating Systems - Lab 3 (Part 1)
 * @brief Two-way Inter-Process Communication (IPC) using pipes and fork().
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 512

int main()
{
    int fd1[2]; // Pipe 1: Parent (P1) -> Child (P2)
    int fd2[2]; // Pipe 2: Child (P2) -> Parent (P1)

    char fixed_str[] = "howard.edu";
    char append_str[] = "gobison.org";
    char input_str[BUFFER_SIZE];
    pid_t p;

    if (pipe(fd1) == -1 || pipe(fd2) == -1)
    {
        fprintf(stderr, "Pipe creation failed\n");
        return 1;
    }

    printf("Input : ");
    fflush(stdout);

    if (scanf("%s", input_str) != 1)
    {
        fprintf(stderr, "Input reading failed\n");
        return 1;
    }

    p = fork();

    if (p < 0)
    {
        fprintf(stderr, "Fork failed\n");
        return 1;
    }

    // Parent process (P1)
    if (p > 0)
    {
        close(fd1[0]); // Close reading end of pipe 1
        close(fd2[1]); // Close writing end of pipe 2

        // Send first input to child
        write(fd1[1], input_str, strlen(input_str) + 1);
        close(fd1[1]);

        // Read modified string back from child via pipe 2
        char final_str[BUFFER_SIZE];
        ssize_t bytes_read = read(fd2[0], final_str, sizeof(final_str) - 1);
        if (bytes_read > 0)
        {
            final_str[bytes_read] = '\0';
        }
        else
        {
            final_str[0] = '\0';
        }
        close(fd2[0]);

        // Wait for child process to terminate
        wait(NULL);

        // Append "gobison.org" safely
        strncat(final_str, append_str, sizeof(final_str) - strlen(final_str) - 1);

        // Print final output
        printf("Output : %s\n", final_str);
    }
    // Child process (P2)
    else
    {
        close(fd1[1]); // Close writing end of pipe 1
        close(fd2[0]); // Close reading end of pipe 2

        // Read string sent by parent
        char concat_str[BUFFER_SIZE];
        ssize_t bytes_read = read(fd1[0], concat_str, sizeof(concat_str) - 1);
        if (bytes_read > 0)
        {
            concat_str[bytes_read] = '\0';
        }
        else
        {
            concat_str[0] = '\0';
        }
        close(fd1[0]);

        // Append "howard.edu"
        strncat(concat_str, fixed_str, sizeof(concat_str) - strlen(concat_str) - 1);

        // Print intermediate output
        printf("Output : %s\n", concat_str);

        // Prompt for second input
        char second_input[BUFFER_SIZE];
        printf("Input : ");
        fflush(stdout);

        if (scanf("%s", second_input) != 1)
        {
            fprintf(stderr, "Second input reading failed\n");
            exit(1);
        }

        // Append second input
        strncat(concat_str, second_input, sizeof(concat_str) - strlen(concat_str) - 1);

        // Send combined string back to parent via pipe 2
        write(fd2[1], concat_str, strlen(concat_str) + 1);
        close(fd2[1]);

        exit(0);
    }

    return 0;
}