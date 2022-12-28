#include <iostream>
#include "ping.h"

int main(int argc, char *argv[]) {
    bool flag = false; // 用于判断是否使用参数 -t
    if (argc < 2) {
        std::cout << "用法：ping [-t] target_name" << std::endl;
        std::cout << "选项：-t \t Ping 指定的主机，直到停止。" << std::endl;
        exit(0);
    }
    else {
        if (std::string(argv[1]) == "-t")
            flag = true;
    }

    MyPing my_ping;

    // 域名解析
    std::string cname;
    cname = flag ? argv[2] : argv[1];
    struct hostent *host = gethostbyname(cname.c_str());
    if (!host) {
        printf("Ping 请求找不到主机 %s。请检查该名称，然后重试。", cname.c_str());
        exit(0);
    }
    const char *ipaddr = inet_ntoa(*(struct in_addr *)host->h_addr_list[0]); // ip地址

    Reply reply{};
    EndInfo info{0, 0, 0, 9999};

    printf("正在 Ping %s [ %s ] 具有 %d 字节的数据\n", cname.c_str(), ipaddr, DEF_PACKET_SIZE);
    int i = 4;
    while (flag || i--) { // 利用短路效应实现-t操作
        info.sent_num++;
        // 利用Ping函数判断是否超时
        if (my_ping.Ping(ipaddr, &reply)) {
            printf("来自 %s 的回复： 字节=%d 时间=%ldms TTL=%ld\n", ipaddr, reply.bytes, reply.rtt, reply.ttl);
            info.min_rtt = std::min<USHORT>(info.min_rtt, reply.rtt);
            info.max_rtt = std::max<USHORT>(info.max_rtt, reply.rtt);
            info.recv_num++;
        }
        else {
            printf("请求超时。\n");
        }
        Sleep(1000);
    }

    // ping请求结束，展示ping统计信息
    printf("\n%s 的 Ping 统计信息:\n", ipaddr);
    printf("\t数据包: 已发送 = %d，已接收 = %d，丢失 = %d，\n", info.sent_num, info.recv_num, info.sent_num - info.recv_num);
    // 只当接收到1个数据包及以上才展示下述信息
    if (info.recv_num > 1) {
        printf("往返行程的估计时间(以毫秒为单位):\n");
        printf("\t最短 = %dms，最长 = %dms，平均 = %dms\n", info.min_rtt, info.max_rtt, (info.min_rtt + info.max_rtt) / 2);
    }

    return 0;
}