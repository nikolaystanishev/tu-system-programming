#include<unistd.h>
#include<signal.h>
#include<stdio.h>
#include<time.h>

#define TIMEOUT 10

void HandleTimeout(int sig) 
{
	time_t timer;
	time(&timer);

	printf("%s\n", ctime(&timer)); // Print current date.
	alarm(TIMEOUT); // Set timeout
}

int main() 
{
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_handler = HandleTimeout; // Function pointer to handler.
	act.sa_flags = SA_RESTART;

	if(sigaction(SIGALRM, &act, 0) == -1)
	{
		perror("sigaction");
		return (1);
	}
	
	if(alarm(TIMEOUT) == -1)
	{
		perror("alarm");
	}
	else
	{
		for(;;)
		{
			pause(); // Waits for signal.
		}
	}

	return 0;
}