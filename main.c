#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "sync.h"

#define BASH_COMMAND_NOT_FOUND (127)
#define MAX_TASK (4)

int main()
{
	pid_t pids[MAX_TASK];
	int weight[MAX_TASK] = { 100, 250, 500, 750 };
	pid_t pid;

	TELL_WAIT();
	for (int i = 0; i < MAX_TASK; i++) {
		if ((pid = fork()) < 0) { /* fork failed */
			fprintf(stderr, "Fork failed...\n");
		} else if (pid == 0) { /* child process */
			char file_name[FILENAME_MAX] = { 0 };
			int fd = 0;

			WAIT_PARENT(); /* wait parent's start signal */

			sprintf(file_name, "output_%d_%d.log", getpid(),
				weight[i]);
			fd = open(file_name, O_CREAT | O_WRONLY, 0755);
			dup2(fd, STDOUT_FILENO); /* redirect */
			close(fd);
			if (execlp("echo", "echo", file_name, (char *)0) < 0) {
				fprintf(stderr,
					"execlp running failed...(errno: %d)\n",
					errno);
				return errno;
			}
		}
		/* parent process */
		pids[i] = pid;
	}
	/* parent process */
	for (int i = 0; i < 4; i++) { /* Start trace notify */
		TELL_CHILD(pids[i]);
	}

	for (int i = 0; i < 4; i++) {
		int status = 0;
		if ((pid = waitpid(pids[i], &status, 0)) < 0) {
			fprintf(stderr, "wait pid failed...\n");
		}
		fprintf(stdout, "process terminated detected %d\n", pid);
	}

	return 0;
}