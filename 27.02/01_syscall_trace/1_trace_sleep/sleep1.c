#include <unistd.h>	// sleep
#include <signal.h>	// signal
#include <stdio.h>	// printf

void handler(int s)
{
    printf("Signal %d\n", s);
}

int main()
{
    signal(SIGINT, handler);
    signal(SIGUSR1, handler);
    int ret = sleep(20);

    return ret;
}
