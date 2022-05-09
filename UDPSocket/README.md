# UDP socket 编程
UDP(User Datagram Protocol)用户数据报协议。相较于TCP，UDP是一种无连接的不可靠的传输协议，但正是因为不可靠，其传输效率要高于TCP。应用场景有DNS(域名解析系统)、NFD(网络文件系统)、SNMP(简单网络管理协议)

## UDP程序结构

### 客户端
1. 调用`socket`函数创建socket；
2. 调用`sendto`函数发送数据，`recvfrom`函数接收数据；
3. 数据传输完成，调用`close`关闭socket；

### 服务端
1. 调用`socket`函数创建socket；
2. 调用`bind`函数将socket绑定至端口；
3. 调用`recvfrom`函数接收数据，`sendto`函数发送数据；


## `recvfrom` 、`sendto` 函数
这两个函数与TCP编程用到的`read`、`write`函数作用相同，但是函数的声明不同，如下所示：
```c
#include <sys/socket.h>

ssize_t recvfrom(int sockfd, void* buff, size_t nbytes, int flag, struct sockaddr* from, socklen_t* addrlen);

ssize_t sendto(int sockfd, void* buff, size_t nbytes, int flag, const struct sockaddr* to, socklen_t addrlen);
```
参数`sockfd`指明了描述符、参数`buff`指明了写入、读出的缓冲区、参数`nbytes`指明了读取、写入的数据的字节数

`flag`参数通知了内核该连接的某些特性，取值范围为0或某些特定常数值用以指定相应的特性。0标识无特殊设置。

`from`和`to`参数分别指向了数据包的发送者、接收者的socket地址。`addrlen`参数指明了socket地址结构体的长度。

以上参数中，`recvfrom`函数的`from`、`addrlen`是值结果参数。


## UDP的相关问题
在程序`udpClient.c`中连接了服务器的标准`echo`服务，在该服务未开启时，客户端发送数据时不会得到回应。这是因为`sendto`函数引发了一个异步的ICMP错误，但是该错误却不会通知给客户端进程。因此客户端程序会一直阻塞于`recvfrom`调用上。

### UDP的`connect`函数
在UDP协议场景下调用`connect`函数并不会建立连接(UDP是面向无连接的)，而是会检查是否存在立即可知的错误，并将相关的信息(对端地址)返回给调用进程。确保在出现错误的情况下即使进行处理。

```c
int sockfd = Socket(AF_INET, SOCK_DRGAM, 0)
Connect(sockfd, (SA*)peeraddr, servlen);//peeraddr是对端的socket地址，servlen是对端socket地址结构的长度
```
采用如上形式对udp的socket调用`connect`函数

对于调用了`connect`函数的udp socket，其与默认的udp socket有着以下几点的差异：

1. 不能继续使用`sendto`函数来指定对端的socket协议地址，因为`connect`已经进行地址的绑定，所有该udp socket上的待发送数据都会自动地向指定的socket协议地址发送。`sendto`函数的对端socket协议地址指针参数必须为`NULL`，协议地址长度参数必须为0。或者也可以使用`write`、`send`函数来执行数据的发送；
2. udp socket能够进行数据交换的对端被限制为`connect`函数指定的那一个，因此不必再采用`recvfrom`函数来获取数据报的发送者，因为它是已知的；
3. 由已连接udp socket引发的异步错误会返回给所在进程

对于已经建立了连接的udp socket，可以通过多次调用`connect`函数来更改对端的socket 协议地址或者断开连接。

更改udp连接的socket协议地址的`connect`的调用方式与首次建立连接的调用方式相同，但是断开连接的`connect`函数需要指定首次建立连接时的socket协议地址的地址族为`AF_UNSPEC`，然后再调用`connect`即可(断开连接的方式可能随系统不同而变化，具体参考系统的实现)。

<font color=#FF000>采用connect还会降低udp发送数据报过程中的开销：</font>

没有建立连接的udp socket调用`sendto`函数发送数据报时，执行的流程如下：

1. 内核暂时连接udp socket
2. 发送第一个数据报
3. 断开udp socket的连接
4. 内核再次连接udp socket
5. 发送第二个数据报
6. 断开内核与udp socket的连接

每发送一个udp数据报都要先建立内核与udp socket的连接，而临时连接未连接udp socket会耗费每个udp传输大约三分之一的开销。

当进程需要一次发送多个数据报给对端的话，建立连接将是较为高效的方式。


在建立了连接之后，内核会为udp socket选定一个IP地址(假设有多个IP)，并且会搜索路由表来决定本地IP与对端IP之间的接口，同时`connect`函数还会为本地的socket指定一个临时端口。