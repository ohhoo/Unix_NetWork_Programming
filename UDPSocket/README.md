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
