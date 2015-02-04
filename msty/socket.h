#include <winsock2.h>
#define _YSHISOCKET_
namespace msty
{
int WSAStart()
{

    WSAData wsaData;
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
    {
        return 255;
    }
    return 1;
}
}
