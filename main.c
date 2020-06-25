#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "sync.h"

#define MAX_TASK (4)
#define MAX_COMMAND_SIZE (1024)

int read_config_file(const char *file_name, int weight[],
		     char bench_file[MAX_TASK][FILENAME_MAX]);
int bench_exec_process(int weight, char bench_file[]);

int main()
{
	pid_t pids[MAX_TASK];
	int weight[MAX_TASK] = { 100, 250, 500, 750 };
	char bench_file[MAX_TASK][FILENAME_MAX];
	pid_t pid;
	int ret;

	if (getuid() != 0) {
		fprintf(stderr,
			"You have to run this program with superuser privilege\n");
		return -EINVAL;
	}

	memset(bench_file, 0, sizeof(bench_file));

	ret = read_config_file("config.txt", weight, bench_file);
	if (ret < 0) {
		fprintf(stderr, "File has invalid data....\n");
		return ret;
	}

	TELL_WAIT();
	/* execute the fork sequence... */
	for (int i = 0; i < MAX_TASK; i++) {
		if ((pid = fork()) < 0) { /* fork failed */
			fprintf(stderr, "Fork failed...\n");
		} else if (pid == 0) { /* child process */
			if (bench_exec_process(weight[i], bench_file[i])) {
				return errno;
			}
		}
		/* parent process */
		pids[i] = pid;
	}

	/* parent process */
	for (int i = 0; i < 4; i++) { /* Start trace notify */
		char buffer[sysconf(_SC_ARG_MAX)];
		sprintf(buffer, "touch sample%d.txt", i);
		system(buffer);
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

int read_config_file(const char *file_name, int weight[],
		     char bench_file[MAX_TASK][FILENAME_MAX])
{
	FILE *fp = NULL;
	int ret = 0;
	fp = fopen(file_name, "r");
	if (fp != NULL) {
		for (int i = 0; i < MAX_TASK; i++) {
			if (feof(fp)) {
				ret = -EFAULT;
				break;
			}
			fscanf(fp, "%d %s", &weight[i], bench_file[i]);
			fprintf(stdout, "weight: %d, file: %s\n", weight[i],
				bench_file[i]);
		}
		fclose(fp);
	} else {
		fprintf(stdout, "cannot find config file...\n");
	}
	return ret;
}

int bench_exec_process(int weight, char bench_file[])
{
	char file_name[FILENAME_MAX] = { 0 };
	int fd = 0;

	WAIT_PARENT(); /* wait parent's start signal */
	printf("%d: %d\n", getpid(), getuid());
	sprintf(file_name, "%d_%d_%s.log", getpid(), weight, bench_file);
	fd = open(file_name, O_CREAT | O_WRONLY, 0755);
	dup2(fd, STDOUT_FILENO); /* redirect */
	close(fd);
	if (execlp("echo", "echo", file_name, (char *)0) < 0) {
		fprintf(stderr, "execlp running failed...(errno: %d)\n", errno);
		return errno;
	}
	return 0;
}
