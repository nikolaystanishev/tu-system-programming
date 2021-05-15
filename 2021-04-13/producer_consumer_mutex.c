#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUF_SIZE 50

void *consumer(void *params);
void *producer(void *params);

char  buffer[BUF_SIZE];
char  full_flag = 0;
char  thr_exit  = 0;
pthread_mutex_t lock;

int main()
{
	printf("main");
	pthread_t tid_prod;
	pthread_t tid_cons;

	if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
	else
	{
		pthread_create(&tid_prod, NULL, producer, NULL);
		pthread_create(&tid_cons, NULL, consumer, NULL);

		pthread_join(tid_prod, NULL);
		pthread_join(tid_cons, NULL);

		pthread_mutex_destroy(&lock);
		pthread_exit(NULL);
	}

	return 0;
}

void *consumer(void *params)
{
	while (!thr_exit)
	{
		pthread_mutex_lock(&lock);

		if (1 == full_flag)
		{
			// Print to the terminal (1 is the FS of the output to terminal).
			(void) write(1, buffer, sizeof(buffer));
			full_flag = 0;
		}

		pthread_mutex_unlock(&lock);
		sched_yield(); // usleep(10)
	}

	pthread_mutex_unlock(&lock);
	pthread_exit(0);
}

void *producer(void *params)
{
	while (1)
	{
		pthread_mutex_lock(&lock);

		if (0 == full_flag)
		{
			// Receive from the terminal (0 is the FS of the input from terminal).
			(void) read(0, buffer, sizeof(buffer) > 0);

			if(feof(stdin))
			{
				thr_exit = 1;
				pthread_mutex_unlock(&lock);

				pthread_exit(0);
			}

			full_flag = 1;
		}

		pthread_mutex_unlock(&lock);
		sched_yield(); // usleep(10)
	}
}
