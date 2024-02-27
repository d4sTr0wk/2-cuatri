#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#define SLEEP_TIME 20

void	handler(int sn)
{
	printf("recibida: %d\n", sn);

	return;
}

int	main(void)
{
	signal(SIGINT, handler);

	printf("inicio: %d\n", getpid());
	int err = sleep(SLEEP_TIME);
	if (err != 0) perror("sleep");
	
	return (err);
}
