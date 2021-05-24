#include<unistd.h>
#include<signal.h>
#include<stdio.h>
#include<sys/time.h>

#define TIMEOUT 5

void HandlerTimeout(int sig) 
{
	time_t timer;
	time(&timer);
	printf("%s\n", ctime(&timer));
}

int main()
{
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_handler = HandlerTimeout;
	act.sa_flags = SA_RESTART;
	if(sigaction(SIGALRM, &act, 0) == -1)
	{
		perror("sigaction");
		return(1);
	}

	struct itimerval val;
	val.it_interval.tv_sec = TIMEOUT;
	val.it_interval.tv_usec = 0;
	val.it_value.tv_sec = TIMEOUT;
	val.it_value.tv_usec = 0;

	if(setitimer(ITIMER_REAL, &val, 0) == -1)
	{
		perror("setitimer");
		return (1);
	}

	else
	{
		for(;;)
		{
			pause();
		}
	}

	return 0;
}