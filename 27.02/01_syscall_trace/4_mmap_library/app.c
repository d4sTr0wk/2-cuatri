#include "naivelib.h"
#include <stdlib.h>

int main(int argc, char* argv[])
{
    do {
        naivelib_foo(*argv);
    }while (*++argv);

    exit(0);
}
