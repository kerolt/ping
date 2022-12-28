#include <iostream>
#include "ping.h"

USHORT MyPing::packet_seq_ = 0;

MyPing::MyPing() : icmp_data_(nullptr), is_init_(FALSE), send_timestamp_(0) {
    WSADATA WSAData;
    if (WSAStartup(MAKEWORD(1, 1), &WSAData) != 0) {
        printf("初始化错误！ %d\n", GetLastError());
        return;
    }
    event_ = WSACreateEvent();
    curr_proc_id_ = (USHORT)GetCurrentProcessId();

    socket_ = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, nullptr, 0, 0);
    if (socket_ == INVALID_SOCKET) {
        printf("无效的socket！%ld\n", WSAGetLastError());
    }
    else {
        WSAEventSelect(socket_, event_, FD_READ);
        is_init_ = TRUE;
        icmp_data_ = (char *)malloc(DEF_PACKET_SIZE + sizeof(ICMPHeader));
        if (icmp_data_ == nullptr) {
            is_init_ = FALSE;
        }
    }
}

MyPing::~MyPing() {
    WSACleanup();
    if (icmp_data_ != nullptr) {
        free(icmp_data_);
        icmp_data_ = nullptr;
    }
}

BOOL MyPing::Ping(const char *dest_ip, Reply *reply, DWORD timeout) {
    if (dest_ip == nullptr) {
        return FALSE;
    }
    // 判断初始化是否成功
    if (!is_init_) {
        return FALSE;
    }

    sockaddr_in dest_sockaddr{};
    dest_sockaddr.sin_family = AF_INET;
    dest_sockaddr.sin_addr.s_addr = inet_addr(dest_ip); // ip地址转换

    bool send = SendEchoRequest(dest_sockaddr);
    if (!send) {
        printf("发送出错！\n");
    }
    bool recv = RecvEchoReply(dest_sockaddr, packet_seq_, reply, timeout);
    return recv;
}

bool MyPing::SendEchoRequest(sockaddr_in dest_sockaddr) {
    // 构建ICMP包
    int icmp_size = DEF_PACKET_SIZE + sizeof(ICMPHeader);
    send_timestamp_ = GetTickCountCalibrate();
    USHORT seq = ++packet_seq_;
    memset(icmp_data_, 0, icmp_size);
    ICMPHeader *icmp_header = (ICMPHeader *)icmp_data_;
    icmp_header->type = ECHO_REQUEST;
    icmp_header->code = 0;
    icmp_header->id = curr_proc_id_;
    icmp_header->seq = seq;
    icmp_header->timestamp = send_timestamp_;
    icmp_header->checksum = CalCheckSum((USHORT *)icmp_data_, icmp_size);

    // 向目的IP发送ICMP报文
    int dest_sockaddr_size = sizeof(dest_sockaddr);
    if (sendto(socket_, icmp_data_, icmp_size, 0, (struct sockaddr *)&dest_sockaddr, dest_sockaddr_size) == SOCKET_ERROR) {
        return false;
    }
    return true;
}

bool MyPing::RecvEchoReply(sockaddr_in dest_sockaddr, int seq, Reply *reply, DWORD timeout) {
    char recv_buf[256] = {"\0"};
    // 接收响应报文
    while (TRUE) {
        ULONG recv_timestamp = GetTickCountCalibrate(); // 获取回复报文的到达时间
        int dest_sockaddr_size = sizeof(dest_sockaddr);
        int packet_size = recvfrom(socket_, recv_buf, 256, 0, (struct sockaddr *) &dest_sockaddr, &dest_sockaddr_size);
        if (packet_size != SOCKET_ERROR) {
            IPHeader *ip_header = (IPHeader *) recv_buf;
            USHORT ip_header_len = (USHORT) ((ip_header->version_head_len & 0x0f) * 4);
            ICMPHeader *pICMPHeader = (ICMPHeader *) (recv_buf + ip_header_len);

            if (pICMPHeader->id == curr_proc_id_    // 是当前进程发出的报文
                && pICMPHeader->type == ECHO_REPLY  // 是ICMP响应报文
                && pICMPHeader->seq == packet_seq_  // 是本次请求报文的响应报文
                    ) {
                reply->seq = seq;
                reply->rtt = recv_timestamp - pICMPHeader->timestamp;
                reply->bytes = packet_size - ip_header_len - sizeof(ICMPHeader);
                reply->ttl = ip_header->ttl;
                return true;
            }
        }
        if (GetTickCountCalibrate() - send_timestamp_ >= timeout) {
            return false;
        }
    }
}

USHORT MyPing::CalCheckSum(USHORT *buffer, int size) {
    unsigned long check_sum = 0;
    while (size > 1) {
        check_sum += *buffer++;
        size -= sizeof(USHORT);
    }
    if (size) {
        check_sum += *(UCHAR *)buffer;
    }

    check_sum = (check_sum >> 16) + (check_sum & 0xffff);
    check_sum += (check_sum >> 16);

    return (USHORT)(~check_sum);
}


ULONG MyPing::GetTickCountCalibrate() {
    static ULONG first_call_tick = 0;
    static LONGLONG first_call_tick_ms = 0;

    SYSTEMTIME sys_time;
    FILETIME filetime;
    GetLocalTime(&sys_time);
    SystemTimeToFileTime(&sys_time, &filetime);
    LARGE_INTEGER curr_time;
    curr_time.HighPart = filetime.dwHighDateTime;
    curr_time.LowPart = filetime.dwLowDateTime;
    LONGLONG curr_time_ms = curr_time.QuadPart / 10000;

    if (first_call_tick == 0) {
        first_call_tick = GetTickCount();
    }
    if (first_call_tick_ms == 0) {
        first_call_tick_ms = curr_time_ms;
    }

    return first_call_tick + (ULONG)(curr_time_ms - first_call_tick_ms);
}