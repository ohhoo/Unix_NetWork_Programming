# 基本TCP socket编程
TCP( Transmission Control Protocol ) 传输控制协议，是面向连接的、可靠的、基于字节流的传输层通信协议。

在进行TCP socket编程时，以下几个函数是必须调用的。

## socket 函数
一个TCP网络程序通常都是从调用`socket`函数开始的
```c
#include <sys/socket.h>
int socket(int family, int type, int protocol)
```
`family`指明协议族(IPv4或IPv6)，`type`指明套接字类型，`protocol`指明协议类型<br></br>

`family`取值范围为：`AF_INET` IPv4协议、`AF_INET6` IPv6协议、`AF_LOCAL` Unix域协议、`AF_ROUTE` 路由socket、`AF_KEY` 密钥socket；

`type`的取值范围为：`SOCK_STREAM` 字节流socket、`SOCK_DGRAM` 数据报socket、 `SOCK_SEQPACKET` 有序分组socket、`SOCK_RAW` 原始socket；

`protocol`的取值范围为：`IPPROTO_TCP` TCP传输协议、`IPPROTO_UDP` UDP传输协议、`IPPRPTO_SCTP` SCTP传输协议；

当`protocol`参数为`IPPROTO_TCP`时，`type`的值只能为`SOCK_STREAM`，因为TCP协议是基于字节流的协议。

当`socket`函数调用成功时，将返回一个小的非负整数值来描述socket，称为socket描述符`sockfd`

## connect 函数
该函数的作用是建立与服务器的连接。
```c
#include <sys/socket.h>
int connect(int sockfd, const struct sockaddr* servaddr, socklen_t addrlen);
```
`sockfd`是socket描述符，`servaddr`是指向socket结构的指针，`addrlen`是socket结构的大小<br></br>
