# 本章节介绍在Unix环境下进行网络编程时会用到的API
## socket地址结构
IPv4版本
```c
struct in_addr{
    in_addr_t s_addr;   //实际是一个32位无符号int型数据，存储32位的网络地址
}

struct sockaddr_in{
    uint8_t sin_len;        //无符号的8bit char 表示该结构的长度
    sa_family_t sin_family; //无符号的short 8bit 表示socket版本
    int_port_t sin_port;    //16bit的端口号
    struct s_addr sin_addr; //IP地址
    char sin_zero[8];       //未曾使用
}
```
需要注意的是tcp、udp端口号在socket地址结构中，总是以<font color=#FF000>网络字节序(即高位字节存放到低位地址上)</font>来存储的

IPv6版本
```c
struct in6_addr{
    uint8_t s6_addr[16] //128bit的IPv6地址信息
}

#define SIN6_LEN

struct sockaddr_in6{
    uint8_t sin6_len;           //该结构体的长度
    sa_family_t sin_family;     //协议类型
    in_port_t sin6_port;        //传输层端口
    uint32_t sin6_flowinfo;     //流标字段，实际中并未使用
    struct in6_addr sin6_addr;  //IPv6地址
    uint32_t sin6_scope_id;     //如果该地址具备范围，则标识其范围
}

```

通用socket地址结构
```c
struct sockaddr{
    uint8_t sa_len;
    sa_family_t sa_family;
    char sa_data[14];   //特定协议地址，包括了目标地址与端口地址
}
```
可以将不同类型的socket地址类型转换为通用socket，然后作为函数的参数使用，但是`sockaddr`存在的缺点是将目标主机地址与端口信息混在一起.<br></br>

在Unix中进行网络编程使用socket结构时，需要将socket的长度信息一同传递，通常使用指针来传递socket地址结构，而对于socket地址结构的长度，则有不同的传递方式,这需要根据数据传递的方向来决定: 
1. 当从进程向内核传递socket结构时，内核需要知道应该从进程中复制多少数据到内核中，因此将socket结构的长度作为一个<font color=#FF00>值</font>进行传递，使用值传递的方式。从进程向内核传递数据的函数包括：
   ```c
   int bind(sockfd,(SA*)&serv,sizeof(serv));
   int connect(sockfd, (SA*)&serv, sizeof(serv));
   int sendto(sockfd, const void* msg, int len, unsigned int flags, const struct sockaddr* to, sizeof(to));
   ```
2. 当从内核向进程中传递socket结构时，内核需要向进程传递它在socket中写入了多少数据，避免进程读取时越界，但是函数的返回值通常需要传递其他信息，因此需要将数据长度存储在指针指向的地址中进行传递。从内核向进程传递socket地址结构的函数包括
   ```c
   int accept(int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen);
   int recvfrom(int sockfd, void *buff, size_t nbytes, int flags, struct sockaddr *from, socklen_t *addrlen);
   int getsockname(int sockfd, struct sockaddr *localaddr, socklen_t *addrlen);
   int getpeername(int sockfd, struct sockaddr *peeraddr, socklen_t *addrlen)
   ```