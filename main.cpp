#include <winsock2.h>
#include <stdio.h>
#include "ping.h"

int main(int argc, char *argv[])
{
    CPing objPing;

    const char *szDestIP = "liuyx.cc";
    printf("%s\n", gethostbyname("liuyx.cc")->h_name);

    PingReply reply;

    printf("Pinging %s with %d bytes of data:\n", szDestIP, DEF_PACKET_SIZE);
    while (TRUE)
    {
        objPing.Ping(szDestIP, &reply);
        printf("Reply from %s: bytes=%d time=%ldms TTL=%ld\n", szDestIP, reply.m_dwBytes, reply.m_dwRoundTripTime, reply.m_dwTTL);
        Sleep(1000);
    }

    return 0;
}