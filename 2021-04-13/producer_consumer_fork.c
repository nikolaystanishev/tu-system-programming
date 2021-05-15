#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUF_SIZE 50
#define FIFO_NAME "pipe_fifo"

void consumer(int);
void producer(int);

char  buffer[BUF_SIZE];
char  full_flag = 0;
char  thr_exit  = 0;
sem_t * sem;

int main()
{
	printf("main");

	pid_t pid;
	int named_pipe = 0;

	if ((sem = sem_open("sem", O_CREAT | O_EXCL, 0644, 1)) == SEM_FAILED) {
		perror("semaphore initilization");
	}
	else
	{
		sem_unlink("sem");

		if(mkfifo(FIFO_NAME, 0666) != 0) {
			perror("pipe creation");
		}
		pid = fork();
		if (pid < 0) {
            perror("Fork error.\n");
		}

		if (pid != 0) {
			named_pipe = open(FIFO_NAME, O_RDONLY);
			producer(named_pipe);
			close(named_pipe);
		} else {
			named_pipe = open(FIFO_NAME, O_WRONLY);
			consumer(named_pipe);
			close(named_pipe);
		}

		sem_destroy(sem);
		pthread_exit(NULL);
	}

	return 0;
}

void consumer(int named_pipe)
{
	while (!thr_exit)
	{
		sem_wait(sem);

		if (1 == full_flag)
		{
			// Print to the terminal (1 is the FS of the output to terminal).
			(void) read(named_pipe, buffer, sizeof(buffer) > 0);
			(void) write(1, buffer, sizeof(buffer));
			full_flag = 0;
		}

		sem_post(sem);
		sched_yield(); // usleep(10)
	}

	sem_post(sem);
	pthread_exit(0);
}

void producer(int named_pipe)
{
	while (1)
	{
		sem_wait(sem);

		if (0 == full_flag)
		{
			// Receive from the terminal (0 is the FS of the input from terminal).
			(void) read(0, buffer, sizeof(buffer) > 0);
			(void) write(named_pipe, buffer, sizeof(buffer));

			if(feof(stdin))
			{
				thr_exit = 1;
				sem_post(sem);

				pthread_exit(0);
			}

			full_flag = 1;
		}

		sem_post(sem);
		sched_yield(); // usleep(10)
	}
}
