#include <signal.h>	// signal
#include <stdio.h>	// printf
#include <errno.h>	// errno

void handler(int s)
{
    printf("Signal %d\n", s);
}

int main()
{
    signal(SIGINT, handler);
    signal(SIGUSR1, handler);

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
