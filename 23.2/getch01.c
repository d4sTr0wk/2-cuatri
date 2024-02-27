#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

#define SLEEP_TIME 20

void handler_siginfo(int s)
{
	printf("Recibida: %d\n", s);
	return;
}

int main(void)
{
	struct sigaction sa;
	sa.sa_sigaction = handler_siginfo;
	sa.sa_flags = 0; //SA_RESTART;
	sigaction(SIGINT, &sa, NULL);

	int c = getchar();
	if (c == EOF) // En caso de error devuelve EOF que equivale a -1
	{
		if (errno != 0)
			perror("getchar");
	}
	else
		printf("Character: %c", c);

	return (0);
}
