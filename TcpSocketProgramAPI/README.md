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
`sockfd`是socket描述符，`servaddr`是指向socket结构的指针，该指针指向的socket地址结构中必须包含IP地址与端口号，`addrlen`是socket结构的大小

当`connect`函数调用成功后，会返回0，否则返回-1

对于TCP socket，当`connect`函数调用时，内核会开始建立TCP连接(连接的建立完全由内核完成)。从连接发起的角度来看，整个连接建立的过程如下：
```
1、客户端发送一个SYN(同步)分节，其中包含了客户将在连接中发送的数据的初始序列号；

2、服务器对客户端发送的SYN分节进行确认，发送一个ACK，同时也要发送自己的SYN(同步)分节；

3、客户端确认服务器的SYN分节，即发送一个ACK
```

当`connect`函数调用失败后，`sockfd`对应的将不可再用，因此在`connect`调用失败后都必须手动关闭对应的socket，并重新调用`socket`函数生成新的socket。


## bind 函数
`bind`函数将本地协议地址赋予一个socket，协议地址是IPv4地址或IPv6地址与端口号的复合。
```c
#include <sys/socket.h>
int bind(int sockfd, const struct sockaddr* myaddr, socklen_t addrlen);
```
参数`sockfd`表示一个socket描述符，参数`myaddr`是一个指向特定于协议的地址的指针，参数`addrlen`表示协议地址结构的长度。

当进行绑定时，IP地址与端口号都是可选项，可以填写也可以不写。

在TCP客户端上，当调用`connect`函数后内核开始建立连接，此时内核会为socket选择一个临时的端口。对于IP地址，在TCP客户端上可以将IP绑定到socket上，这就为在该socket上发送的IP数据报指定了源IP地址，当没有绑定IP地址时，内核将会选择外出网络接口作为该socket的绑定IP。


在TCP服务器上，对于端口号，虽然内核也会自动地选择一个临时端口，但是由于服务器通过端口来标识一个服务，因此最好是指定一个端口号。对于IP地址，将一个IP地址绑定到一个socket上时，则说明该socket只接受以该IP作为目的地址的客户连接。如果没有绑定IP地址，内核就将客户端发送SYN分节的目的IP作为服务器的源IP地址。

<font color=#FF000>因此，对于`bind`函数可以指定IP地址与端口号，也可以不指定。如果不指定端口号，则会在`bind`函数调用时选择一个临时端口,该端口号无法通过参数进行返回;而如果不指定IP地址，则会在连接建立(TCP)或者数据报发出时(UDP)选择一个IP地址</font>如果需要获取临时选择的IP、端口号信息，则需要调用`getsockname`函数；

