#include <arpa/inet.h>
#include <stdio.h>

/*
sockaddr 是一个通用的套接字地址结构，它通常与特定的地址族结构（如 sockaddr_in ）一起使用。
这是因为多数套接字函数，如 bind(), connect(), 和 accept()，需要使用指向 sockaddr 结构的指针的参数。
*/
// struct sockaddr
// {
//     sa_family_t sa_family; /* Address family */
//     char sa_data[];        /* Socket address */
// };

// // 套接字地址结构（适用于IPv4网络通信的地址结构）
// struct sockaddr_in
// {
//     sa_family_t sin_family;  /* address family: AF_INET */
//     in_port_t sin_port;      /* port in network byte order */
//     struct in_addr sin_addr; /* ip address */
// };

// struct in_addr
// {
//     uint32_t s_addr; /* address in network byte order */
// };

/*
网络地址转换函数 (用于将IP地址在可打印的格式和二进制结构之间转换)
将点分十进制的IP地址（如"192.168.1.1"）转换成网络字节顺序的二进制形式
inet_pton()
将网络字节顺序的二进制IP地址转换为点分十进制字符串格式
inet_ntop()
*/

int main()
{
#define INET_ADDRSTRLEN 16

    char str[INET_ADDRSTRLEN];
    struct in_addr ipv4addr;
    inet_pton(AF_INET, "192.168.1.1", &ipv4addr);
    inet_ntop(AF_INET, &ipv4addr, str, INET_ADDRSTRLEN);
    printf("The IPv4 address is: %s\n", str);

    return 0;
}