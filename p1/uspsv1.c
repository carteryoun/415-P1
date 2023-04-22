#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include "p1fxns.h"

#define MAX_ARGC 50
#define MAX_ARGV 100

int main(int argc, char *argv[]) {
	char *line = NULL;
	size_t len = 0;
	ssize_t nread;
	FILE *file = stdin;
	int numprograms = 0;
	char *program[MAX_ARGC];
	char *args[MAX_ARGC][MAX_ARGV];
	int pid[MAX_ARGC];

	// Open a file or use stdin
	if (argc > 1) {
		file = open(argv[1], O_RDONLY); // change from fopen to something else
		if (!file) {
			perror("open");
			exit(EXIT_FAILURE);
		}
	}

	// Read program workload
	while ((nread = p1getline(&line, &len, file)) != -1) {
		// Parse program and arguments
		int argc = p1getword(line, 0, &program[numprograms]);
		for(int i = 0; i < argc; i++) {
			p1getword(line, i+1, &args[numprograms][i]);
		}
		args[numprograms][argc] = NULL; // Null-terminate arg list
		numprograms++;
	}
	free(line);
	close(file);

	// Launch programs
	for (int i = 0; i < numprograms; i++) {
		pid[i] = fork();
		if(pid[i] == -1) {
			perror("fork");
			exit(EXIT_FAILURE);
		}
		else if (pid[i] == 0) {
			execvp(program[i], args[i]);
			perror("execvp");
			exit(EXIT_FAILURE);
		}
	}

	// Wait for all programs to finish, then terminate
	for(int i = 0; i < numprograms; i++) {
		if(waitpid(pid[i], NULL, 0) == -1) {
			perror("waitpid");
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}


// read the program workload from the specified file/stdin
// Each line in the file contains the name of the program
// and its arguments (just as you would present them to bash)




// For each program, launch the program to run as a process using fork(),
// execvp(), and any other system calls. To make things simpler, assume
// that the programs will run in the same env as USPS


// Once all programs are running wait for each process to terminate


// After all processes are terminated, USPS exits