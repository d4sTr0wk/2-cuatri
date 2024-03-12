#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#define SLEEP_TIME 20

void	handler_siginfo(int sn, siginfo_t *si, void *p)
{
	printf("recibida: %d\n", sn);
	signal(SIGINT, SIG_DFL);
	return;
}

int	main(void)
{
	struct sigaction sa;
	sa.sa_handler = handler_siginfo;
	sa.sa_flags = SA_RESTART; // Aunque ponga SA_RESTART Linux no es capaz de retomar el sleep interrumpido
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);

	int err = sleep(SLEEP_TIME);
	if (err != 0) perror("sleep");

	return (err);
}
