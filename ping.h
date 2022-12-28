#pragma once

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib") // 如果是cmake工程则可在CMakeLists.txt中配置lib的链接

#define DEF_PACKET_SIZE 32 // 模拟32字节的数据
#define ECHO_REQUEST 8 // 回显请求
#define ECHO_REPLY 0 // 回显响应

struct IPHeader {
    BYTE version_head_len;          // 4位版本+4位首部长度
    BYTE tos;              // 服务类型
    USHORT total_len;       // 总长度
    USHORT id;             // 标识
    USHORT flag_and_offset; // 3位标志+13位片偏移
    BYTE ttl;              // TTL
    BYTE protocol;         // 协议
    USHORT head_checksum;      // 首部检验和
    ULONG src_ip;           // 32位源IP地址
    ULONG dest_ip;          // 32位目的IP地址
};

struct ICMPHeader {
    BYTE type;       // 类型
    BYTE code;       // 代码
    USHORT checksum; // 检验和
    USHORT id;       // 标识符
    USHORT seq;      // 序号
    ULONG timestamp; // 时间戳（非标准ICMP头部）
};

struct Reply {
    USHORT seq;
    DWORD rtt;
    DWORD bytes;
    DWORD ttl;
};

struct EndInfo {
    USHORT sent_num; // 已发送数据包
    USHORT recv_num; // 已接收数据包
    DWORD max_rtt; //最长往返时间
    DWORD min_rtt; //最短往返时间
};

class MyPing {
public:
    MyPing();
    ~MyPing();
    BOOL Ping(const char *dest_ip, Reply *reply = nullptr, DWORD timeout = 2000); // 实现ping操作

private:
    bool SendEchoRequest(sockaddr_in dest_sockaddr);
    bool RecvEchoReply(sockaddr_in dest_sockaddr, int seq, Reply *reply, DWORD timeout);
    static USHORT CalCheckSum(USHORT *buffer, int size); // 校验和
    static ULONG GetTickCountCalibrate();

private:
    SOCKET socket_;
    WSAEVENT event_;
    USHORT curr_proc_id_;
    char *icmp_data_;
    BOOL is_init_;
    ULONG send_timestamp_;

    static USHORT packet_seq_;
};