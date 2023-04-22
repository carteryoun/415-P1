#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#define UNUSED __attribute__((unused))

volatile int ALRM_seen = 0;

static void onalrm(UNUSED int sig) {
	ALRM_seen++;
}

int main(int argc, char **argv) {
	char* workload[] = {"inbound", "outbound", "mailproc", "logproc", NULL};
	int fd, i, j, status, minutes = 1, number = 6000000;
	char b[1000], pid[10], opt, name[128];
	pid_t pid_arr[4];
	sigset_t mask, oldmask;
	struct sigaction act, oldact;

	// process environment and CL args
	sprintf(name, "%x", getpid());
	opterr = 0;
	while ((opt = getopt(argc, argv, "m:n:")) != -1) {
		switch(opt) {
			case 'm': minutes = atoi(optarg); 
				break;
			case 'n': strcpy(name, optarg);
				break;
			default: exit(EXIT_FAILURE);
		}
	}

	// establish SIGALRM handler
	act.sa_handler = onalrm;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if(sigaction(SIGALRM, &act, &oldact) < 0) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}

	// set alarm in the future
	alarm((unsigned int) 60 * minutes);

	// set up signal mask for waiting

	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigprocmask(SIG_BLOCK, &mask, &oldmask);

	// create child processes
	for (i = 0; i < 4; i++) {
		if((pid_arr[i] = fork() < 0)) {
			perror("fork");
			exit(EXIT_FAILURE);
		} else if (pid_arr[i] == 0){
			// child code
			sigprocmask(SIG_SETMASK, &oldmask, NULL);
			pause();

			// exec workload process
			execlp(workload[i], workload[i], "-n", name, "-m", "1", NULL);
			perror("exec");
			exit(EXIT_FAILURE);
		}
	}

	// wake up child processes
	for(i = 0; i < 4; i++) {
		kill(pid_arr[i], SIGUSR1);
	}

	// wait for children to finish
	for(i = 0; i < 4; i++) {
		waitpid(pid_arr[i], &status, 0);
	}

	// clean
	sigaction(SIGALRM, &oldact, NULL);
	sigprocmask(SIG_SETMASK, &oldmask, NULL);

	return EXIT_SUCCESS;
}





// Immediately after each process is created using fork,
// the child process waits on SIGUSR 1 before calling execvp()


// After all processes have been created and are awaiting the SIGUSR1
// signal, the USPS parent process sends each program a SIGUSR1
// signal to wake them up. Each child process will then invoke execvp()
// to run the workload process


// After all of the processes have been awakened and are executing
// the USPS sends each process as SIGSTOP signal to suspend it


// After all of the workload processes have been suspended, the USPS
// sends each process a SIGCONT signal to resume it


// Once all processes are back up and running, the USPS waits for
// each process to terminate. After all have terminated, USPS exits