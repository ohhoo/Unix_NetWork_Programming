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
对于这种根据传递方向的不同采用不同的传递方式，并承载不同信息的参数有一种称呼为"值-结果"参数，就如同上例中的socket长度参数。<br></br>

## 字节排序函数
tcp、udp端口号在socket地址结构中，总是以<font color=#FF000>网络字节序(即高位字节存放到低位地址上)</font>来存储的，在进行数据传输时，根据高位字节所处的位置，可以将传输顺序分为网络字节序、主机字节序两种，也称为大端字节序与小端字节序<br></br>
假设存在一变量`x`，位于地址`0x100`处，其值为`0x1234567`，地址范围为`0x100~0x103`
1. 大端序的存储方式为：地址`0x100`:数据`0x01`,地址`0x101`:数据`0x23`,地址`0x102`:数据`0x45`,地址`0x103`:数据`0x67`;
2. 小端字节序的存储形式则为：地址`0x100`:数据`0x67`，地址`0x101`:数据`0x45`，地址`0x102`:数据`0x23`，地址`0x103`:数据`0x01`<br></br>

字节序的不同会导致读取到的数据不同，因此需要一些字节排序函数对数据进行不同字节序的转换工作
```c
uint16_t htons(uint16_t host16bitvalue); //执行主机字节序向网络字节序的转换
uint32_t htonl(uint32_t host32bitvalue); //执行主机字节序向网络字节序的转换

uint16_t ntohs(uint16_t net16bitvalue);
uint32_t ntohl(uint32_t net32bitvalue); //执行网络字节序向主机字节序的转换
```
在网络上进行通信时，数据采用网络字节序进行排列存储。如果当前主机使用的字节序是网络字节序，则以上四个函数将会定义为空宏，并不进行实际的数据操作。<br></br>
<font color=#FF000>在进行编程时需要注意网络传输的数据与主机中的数据的字节排列顺序是否存在差别，根据需要进行转换。</font>字节排序函数主要用于socket地址结构中的各个字段。

## 字节操纵函数
这一类的函数主要完成对字节级数据的复制、拷贝等操作；
```c
void bzero(void* dest, size_t nbytes);                          //将目标字节串中指定数目的字节置为0
void bcopy(const void* src, void* dest, size_t nbytes);         //将指定数目字节的数据从源地址拷贝到目标地址
int bcmp(const void* ptr1, const void* ptr2, size_t nbytes);    //对比两个字节串，如果相等则返回0，否则返回非0值

void* memset(void* dest, int c, size_t nbytes);                 //将目标字节串中指定数目的字节置为c 
void* memcpy(void* dest, const void* src, size_t nbytes);       //将源字节串中指定数目的字节拷贝至目标字节串
int memcmp(const void* ptr1, const void* ptr2, size_t nbytes);  //对比任意两个字节串，如果相同则返回0，若ptr1所指字节串中的字节大于ptr2则返回正值，否则返回负值
```
代码中上边部分的是Berkeley版本的字节操纵函数，下边部分的是ANSI C函数，其中`memcpy`在源字节串与目标字节串地址重叠时会出现不可预知的错误，而`bcopy`则可以正确处理，如果需要使用ANSI C函数进行字节串拷贝，则可以使用`memmove`函数。

## 地址转换函数
完成点分十进制字符串的IP地址(便于阅读及输入)与网络字节序的二进制值的IP地址(存储在socket地址结构中)的互相转换
```c
int inet_aton(const char* strptr, struct in_addr* addrptr);//将strptr所指的点分十进制IPv4地址转换为32位的网络字节序地址，存储在in_addr所指的socket地址结构中

in_addr_t inet_addr(const char* strptr)//将点分十进制字符串转换为网络字节序的二进制值，与inet_aton执行相同的转换，但是该函数不能处理255.255.255.255

char* inet_ntoa(struct in_addr inaddr);//返回指向点分十进制字符串的指针

//上面的函数只适用于IPv4的地址转换，以下函数适用与IPv4与IPv6
int inet_pton(int family, const char* strptr, void* addrptr);//将strptr指针所指的字符串地址，转换位二进制结果，存放在addrptr所指的地址中，调用成功则返回1，输入格式有误则为0，出错返回-1

const char* inet_ntop(int family, const void* addrptr, char* strptr, size_t len);//从二进制格式转换为char格式，len表示char格式的大小，正确处理返回指向strprt指针，否则返回NULL
```
`inet_addr`不能处理`255.255.255.255`的原因是当该函数出错时通常返回`INADDR_NONE`(32位全为1的值)


## 字节流socket读写函数
这些函数完成对字节流socket进行读写的操作，包括以下三种函数
```c
//从一个描述符读取n个字节
ssize_t readn(int filedes, void* buff, size_t nbytes);

//向一个描述符中写入n个字节
ssize_t written(int filedes, const void* buff, size_t nbytes);

//从一个描述符读取文本行，一次一个字节
ssize_t readline(int filedes, void* buff, size_t maxlen);//每读取一个字节就调用一次read，执行速度很慢
```
在对socket进行读写的过程中，可能由于socket的缓冲区已经到达了极限，因此会存在一次性不能完全读取所需字节的情况，因此在这三个函数中均会捕获`EINTR`错误，捕获到该错误时会继续进行读写。


## 习题
1. 为什么诸如套接字地址结构的长度之类的值-结果参数要用指针来传递
   
   当从内核向进程中传递socket结构时，内核需要向进程传递它在socket中写入了多少数据，避免进程读取时越界。仅当这种情况下才会使用指针来传递，因为值传递会拷贝变量，不能进行传递，因此需要将数据写入到指针所指的地址中，达到传递消息的目的。

2. 为什么`readn`和`writen`函数都将`void*`型指针转换为`char*`型指针
   
   在读取和写入数据时，是以字节为单位的，而char的大小刚好为一字节，将`void*`转换为`char*`便于读取和写入。同时void* 指针没有类型，无法取值。

3. 试写一个名为`inet_pton_loose`的函数，它能处理以下情形：如果地址族为`AF_INET`且`inet_pton`返回0，那就调用`inet_aton`看是否成功；类似地，如果地址族为`AF_INET6`且`inet_pton`返回0，那就调用`inet_aton`看是否成功，若成功则返回其`IPv4`映射的`IPv6`地址
   
   ```
   int inet_pton_loose(int family, const char* ptrstr, void* addrptr)
   {
       int ret_code;
       if(family == AF_INET){
           //用户指定类型为IPv4，如果不符合，则说明该地址为IPv6，直接从ptrstr中返回该地址
           int code = inet_pton(AF_INET, ptrstr, addrptr);
           if(code == 0){
               //说明是IPv6格式的地址，或错误格式
               ret_code = inet_aton(ptrstr, (struct in_addr*)addrptr);//inet_aton接受in_addr类型的指针，因此需要进行类型转换
           }
       }
       else if(family == AF_INET6){
            //用户指定类型为IPv6，如果不符合则说明该地址为IPv4，转换为IPv6后返回
            int code = inet_pton(AF_INET6, ptrstr, addrptr);
            if(code == 0){
                //说明是IPv4格式的地址，或错误地址
                ret_code = inet_aton(ptrstr, (struct in_addr*)addrptr);
                //将IPv4地址转换为IPv6地址
                char newptr[16];
                bzero(newptr, 10);
                memset(newptr+10, 16, 2);
                memcpy(newptr+12, ptrstr, 4);
                ptrstr = newptr;
            }
       }
       return ret_code;
   }
   ```