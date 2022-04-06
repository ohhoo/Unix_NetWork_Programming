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


## listen 函数
`listen`函数将一个socket转换为一个被动socket，并指示内核应当接受指向该socket的连接。`listen`函数仅由TCP服务器调用
```c
#include <sys/socket.h>
int listen(int sockfd, int backlog);
```
参数`sockfd`是socket描述符，参数`backlog`规定了内核应当为相应socket排队的最大连接数(包括已建立的连接与等待建立的连接)，一般的`backlog`默认为5

`listen`的作用就是在某个socket上监听连接

`backlog`参数规定的最大连接数实际上是要乘以1.5的，即实际允许的已完成连接数与未完成连接数之和大于`backlog`，当一个连接建立后(服务器接收到客户端的ACK分节)后，将该连接从未完成连接队列移至已完成连接队列。当这些队列全满时，如果有客户端的SYN分节到达(建立连接请求)，服务器将会忽略该请求，使客户端触发异常重传，而不是回复一个RST分节(标识异常连接)，是客户明白是由于队列满而无法接受连接请求。

当连接建立后，服务器尚未调用`accept`函数正式接受连接之前，处于连接建立完成队列中的连接可以接受数据，但其能够接受数据的大小为已连接socket的缓冲区的大小。

## accept 函数
该函数在TCP服务器上进行调用，从已完成连接队列的头部返回下一个已完成的连接。当已完成连接队列为空时，进程将进入休眠。
```c
#include <sys/socket.h>
int accept(int sockfd, struct sockaddr* cliaddr, socklen_t* addrlen);
```
参数`sockfd`时监听socket的描述符，参数`cliaddr`与`addrlen`用来返回已连接的对端进程的协议地址，地址信息在`cliaddr`中返回，参数`addrlen`标明地址结构的长度。(当我们对客户端的地址信息不感兴趣时，可以不用获取协议地址信息，将后两个指针类型的参数赋值空指针即可)

当`accept`执行成功后，将返回一个新的socket描述符，该描述符标明了一个`已连接socket`，`accept`从监听socket处获得新的连接，并返回一个全新的`socket`，应当注意这两个`socket`是不同的。


## fork 、 exec 函数
这两个函数提供了服务器并发处理的基本能力，其中`fork`函数是`Unix`中派生新进程的唯一办法，`exec`函数是存放在硬盘上的可执行文件被Unix执行的唯一办法(`exec`函数有六种类型)。
```c
#include <unistd.h>

pid_t fork(void)

int execl(const char* pathname, const char* arg0, ...);

int execv(const char* pathname, char* const *argv[]);

int execle(const char* pathname, const char* arg0, ...);

int execve(const char* pathname, char* const argv[], char* const envp[]);

int execlp(const char* filename, char const *arg0, ...);

int execvp(const char* filename, char* const argv[]);
```
对于`fork`函数，最为特殊的是它调用一次，返回两次，在调用`fork`函数的进程中(父进程)返回一次，本次的返回值是派生的新进程的进程号，在子进程中返回一次，返回值为0。

`fork`在执行新进程创建时，其实是将父进程进行了复制(对内核中描述进程信息的task_struct结构进行了复制)并创建了一个新的进程地址空间与堆栈，这新的进程地址空间与堆栈与父进程使用的实际上是同一个(映射到了同一个物理内存页上)，只有在子进程向进程地址空间中进行写操作时，才会实际地分配一个新的页面给新进程使用(写时拷贝机制)。因此在调用了`fork`后创建的新进程可以通过共享的socket描述符来操作对应的socket。

<font color=#FF000>注意，每一个socket或文件都会有一个引用计数，当进行`fork`后，父进程与子进程共享socket，因此每个socket的引用计数变成了2，所以需要先关闭父进程中对应的socket，这个并不会导致socket实际上被关掉，只是将引用计数减一，只有当引用计数为0时，socket才会被关闭回收。</font>

对于`exec`系列的函数，`execve`函数是系统调用，其余都是调用`execve`的库函数。这几个函数执行的操作都是将当前进程映像替换成新的程序文件，该新程序通常从`main`函数开始执行，在整个过程中不会创建新的进程。这些函数的参数中`pathname`标明了待执行的程序文件路径，而后续的`argv`、`arg0`等参数标明了`pathname`指向的新程序的参数，区别在于这些参数的组织形式是一个个列出还是由数组保存。


## close 函数
`close`函数对一个已经建立的连接进行关闭。
```c
#include <unistd.h>
int close(int sockfd);
```
`close`函数的默认行为是将参数`sockfd`标识的socket标记为已关闭，然后立即返回到调用进程中，该进程不能再向对应的socket写入或者读取数据。当该socket中已经排队等待发送的数据发送完成后，该socket发送FIN分节结束连接。

`close`实际上是将socket的引用计数减一，并不直接使socket发送FIN分节。


## getsockname 、 getpeername 函数
这两个函数用来获取与某个套接字关联的本地协议地址，或者是与某个套接字关联的外地协议地址。
```c
#include <sys/socket.h>
int getsockname(int sockfd, struct sockaddr* localaddr, socklen_t* addrlen);
int getpeername(int sockfd, struct sockaddr* peeraddr, socklen_t* addrlen);
```
这两个函数的后两个参数都是值-结果类型，获取到的协议地址与长度都通过这两个值-结果参数进行返回。

`getsockname`能够获取本地的内核自动选择的IP地址、端口号、地址族信息。
`getpeername`常用于服务器获取客户端的IP地址、端口号信息。

## 习题
1. 我们说头文件`<netinet/in.h>`中定义的`INADDR_`常值是主机字节序的，应当如何辨别？
   ```c
    #define	INADDR_UNSPEC_GROUP	(u_int32_t)0xe0000000	/* 224.0.0.0 */
    #define	INADDR_ALLHOSTS_GROUP	(u_int32_t)0xe0000001	/* 224.0.0.1 */
    #define	INADDR_ALLRTRS_GROUP	(u_int32_t)0xe0000002	/* 224.0.0.2 */
    #define	INADDR_ALLRPTS_GROUP	(u_int32_t)0xe0000016	/* 224.0.0.22, IGMPv3 */
    #define	INADDR_CARP_GROUP	(u_int32_t)0xe0000012	/* 224.0.0.18 */
    #define	INADDR_PFSYNC_GROUP	(u_int32_t)0xe00000f0	/* 224.0.0.240 */
    #define	INADDR_ALLMDNS_GROUP	(u_int32_t)0xe00000fb	/* 224.0.0.251 */
    #define	INADDR_MAX_LOCAL_GROUP	(u_int32_t)0xe00000ff	/* 224.0.0.255 */
   ```
   程序中的定义如上所示，可以看出这是按照主机序定义的。

2. 把图1-5改为在`connect`成功返回后调用`getsockname`。显示赋予TCP socket的本地IP地址与端口号
   
   部分代码如下。
   ```c
    struct sockaddr_in test;
    bzero(&test,sizeof(test));
    socklen_t len = sizeof(test);//当直接声明len而不赋值时，结果均为0，因为默认len为0，所以就不写入信息
    if(getsockname(sockfd, (SA*)& test, &len) < 0>)
        err_sys("error);

    printf("local ip = %d\n", test.sin_addr.s_addr);
    printf("local port = %d\n", test.sin_port);
   ```

3. 在一个并发服务器中，假设`fork`调用返回后，子进程先运行，而且子进程随后在`fork`调用返回父进程之前就完成了对客户的服务。下面程序中的两个`close`调用将会发生什么？
   ```c
   pid_t pid;
   int listenfd,connfd;

   listenfd = Socket(...);
   
   Bind(listenfd,...);
   Listen(listenfd, LISTENQ);

   for(;;){
       connfd = Accept(listenfd,...);
       if( (pid = Fork()) == 0){
           Close(listenfd);
           doit(connfd);
           Close(connfd);
           exit(0);
       }
       Close(connfd);
   }

   ```
   对于`connfd`执行`Close`，第一次将会把`connfd`对应的socket的引用计数减一，但此时`connfd`对应的socket的引用计数不为0，因此不会发送FIN分节关闭连接，而第二次执行`Close`后`connfd`对应的socket引用计数为0，将会发送FIN分级，进入TCP四次挥手断开连接的过程。

4. 在时间服务器程序中，将端口号从13改为9999(这样就不需要超级用户特权就能启动程序)，再删除`listen`调用，将会发生什么？
   ```
   删掉了listen调用，在运行时会出现accept函数执行报错，将不会接收指向fd的连接请求
   ```

5. 继续上一题，删掉了`bind`调用，但是保留`listen`调用，将会发生什么？
   
   ```
   使用getsockname获取listenfd对应的IP地址与端口号，得到：IP地址为0，端口号为26581，这说明系统为监听socket选择了一个随机端口。
   ```