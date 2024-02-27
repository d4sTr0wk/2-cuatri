#include <signal.h>	// signal
#include <stdio.h>	// printf
#include <errno.h>	// errno

void handler(int s)
{
    printf("Signal %d\n", s);
}

int main()
{
    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_flags = 0;	// SA_RESTART is the default when using signal()
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    int ret = getchar();

    if (ret == EOF) {	// End of file OR error
        if (errno == 0) {
            printf("\nBye\n");
        } else {	// error
            perror("getchar");
        }
    } else {
        printf("Char: %c\n", ret);
    }

    return 0;
}
