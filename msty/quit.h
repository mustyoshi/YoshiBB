#include <cstdio>
namespace msty
{
int quit(char*msg)
{
    printf(msg);
    getchar();
#ifdef _YSHISOCKET_hi
    WSACleanup(); //I take care of my st00fs
#endif
    return 1;
}
}
