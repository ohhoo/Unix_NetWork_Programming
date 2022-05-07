# I/O 复用：select和poll

I/O复用是指内核一旦发现进程指定的一个或多个I/O条件就绪，就通知进程。这样就实现了一个进程能够监控多个描述符的功能。

## I/O复用的应用场景
1. 客户处理多个描述符，此时必须使用I/O复用

2. 同时处理多个socket

3. 即处理监听socket又处理已连接socket

4. 既要处理TCP又要处理UDP

5. 一个服务器要处理多个服务或者多个协议

I/O复用不一定局限于网络应用，许多本地应用也可以使用该技术。I/O复用能力是由`select`与`poll`两个函数支持。

## I/O模型
Unix中有5中常见的I/O模型：

1. 阻塞式I/O：当一个进程需要数据输入时，它会一直等待直到数据到达或者发生错误才会返回。

2. 非阻塞式I/O：当所请求的I/O操作需要等待数据到达而一直阻塞时，直接返回一个`EWOULDBLOCK`错误。

3. I/O复用：通过调用`select`或`poll`，进程将阻塞在这两个函数上，而不是阻塞在系统调用上，这样做的好处在于可以同时等待多个描述符就绪。

4. 信号驱动式I/O：进程将不会阻塞与I/O操作，而是直接返回等待I/O条件就绪时内核发送给该进程的信号。

5. 异步I/O：这种情况下进程的I/O操作将会自主执行，只会在I/O操作完成时通知该进程。

## select函数

该函数允许进程指示内核等待多个事件中的一个发生，并在一个或多个事件发生或超时时间到达时唤醒该进程。

```c
#include <sys/select.h>
#include <sys/time.h>

int select(int maxfdp1, fd_set* readset, fd_set* writeset, fd_set* execeptset, const struct timeval* timeout)
```
参数`maxfdp1`标明待测试的描述符的个数，该参数的值是 最大描述符+1，从描述符0开始，到描述符`maxfdp1`-1都会被测试

参数`readset`、`writeset`、`exceptset`分别标明让内核测试的读、写、异常条件的描述符集合。在`select`函数中，使用整数数组来表示这些集合，并且整数的每一位对应一个描述符，比如使用32位整数，那该数组的第一个元素可以标识0~31的描述符，之后依此类推。`fd_set`是系统设置的描述符集合的数据类型，使用方法如下：
```c
fd_set rset;

FD_ZERO(&rset);//初始化，此时所有的描述符都是关闭的

FD_SET(1, &rest);//打开描述符1的对应位
FD_SET(3, &rest);//打开描述符3的对应位
FD_SET(5, &rest);//打开描述符5的对应位
```
<font color=#FF000>描述符集合的初始化是必须的，否则会出现不可预期的结果</font>同时需要注意的是，如果按照上述示例中的描述符调用`select`，`maxfdp1`参数的值应当为6，因为需要的是描述符最大值+1而不是描述符个数+1。

参数`readset`、`writeset`、`exceptset`均为值-结果参数，每次`select`调用返回时，都会将未就绪描述符对应的位置为0，因此在调用`select`之前都要对关心的描述符对应的位数重新进行设置。

参数`timeout`标明了超时时间，`timeval`结构用于指定这段时间的秒数和微秒数
```c
struct timeval{
    long tv_sec;
    long tv_usec;
}
```
`timeout`参数决定了`select`等待的时间：
1. 当该参数传入一个空指针时，`select`函数仅在有一个描述符准备好I/O时才返回，否则会永远地等待
2. 当该参数指向的`timeval`中指定了等待的具体时间时，`select`在有一个描述符准备好I/O或等待时间达到指定时间时返回
3. 该参数指向的`timeval`结构中的秒、微秒数都为0时，`select`不会进行等待，只是对所有描述进行检查后即返回(不管有没有准备就绪的)，这种操作也成为轮询(polling)
   1、2两种情况的等待会被进程在等待期间获取的信号所中断，并结束等待。


## 描述符就绪条件
通常我们需要`select`函数在描述符就绪后返回，但是如何来判定描述符是否就绪？

描述符也分为文件描述符与socket描述符，此处讨论socket描述符的情况，

首先如何判定一个socket可读(进行读操作时不会阻塞)？
1. socket接收缓冲区中的数据字节数大于等于socket接收缓冲区的标记大小，相当于设置了一个最小值，只要缓冲区内数据字节数大于该值，则说明该socket是可读的。此时对socket执行读操作时不会阻塞，而是返回准备好读入的字节值;
2. 该连接接受了FIN分节，不能再从该socket中读取数据，此时读取操作会返回EOF(0);
3. 该socket是一个监听socket，同时已完成的连接数大于0；
4. 该socket中有一个错误需要返回处理，对于该socket的读操作将会返回一个-1，同时将errno设置为确切的错误条件(比如对端的程序崩溃重启，当我们的数据到达时会返回一个RST)；
   
   以上四个条件任意满足一条，对应的socket即可读


如何判断一个socket可写(进行写操作时不会阻塞)？
1. 该socket的发送缓冲区内的可用空间字节数大于规定的最小值，并且该socket已经连接，或者该socket是UDP socket不需要建立连接即可直接向socket中写入数据，这个最小值通常由socket选项`SO_SNDLOWAT`关键字设置，默认值为2048；
2. 该连接的写半部关闭，此时执行写操作将会产生`SIGPIPE`信号；
3. 使用非阻塞式`connetct`的socket已建立连接，或者`connect`以失败告终；
4. 该socket上有错误待处理；

## shutdown
现在考虑一个问题，当输入是批量的，我们的输入与服务端的响应占据了通信管道的全部，因此可能会有部分的输入正在前往服务端的路径上(已经完成了数据的发送)，也可能有服务端的响应在路径上，此时我们可能在接到响应或请求未到达服务端之前就接收到了一个EOF，此时会终止客户端进程，同时客户端连接中可能会有部分数据在缓冲区内无法发送。应当如何解决这个问题？

解决方案之一就是，保持socket描述符继续打开，保证程序仍能够从socket中读取到后续的数据。`shutdown`函数可以完成这一目的。

```c
#include <sys/socket.h>
int shutdown(int sockfd, int howto)
```
参数`sockfd`标明socket描述符，参数`howto`标明如何关闭连接，其取值范围如下：
```c
SHUT_RD    关闭连接的读，socket不能再进行读操作，缓冲区内的数据将被全部丢弃
SHUT_WR    关闭连接的写，socket不能再进行写操作，缓冲区内的数据将被全部发送
SHUT_RDWR  关闭连接的写和读，等效于调用一次SHUT_RD的shutdown后再调用一次SHUT_WR的shutdown
```

此时回答上述提出的问题，当连接在收到了EOF后，调用`shutdown`关闭连接的写部分，此时会将缓冲区内的所有数据发送出去，避免了丢失数据的情况。


## pselect 函数
`pselect` 可以认为是`select`函数的升级版本
```c
#include <sys/select.h>
#include <signal.h>
#include <time.h>

int pselect(int maxfdp1, fd_set* readset, fd_set* writeset, fd_set* execptset, const struct timespec* timeout, const sigset_t* sigmask);
```

参数`maxfdp1`、`readset`、`writeset`、`execptset`与`select`的相应参数取值范围、功能等一致。

不同之处在于表示时间的参数`timeout`，采用了不同的结构类型
```c
struct timespec{
   time_t tv_sec; //秒
   long tv_nsec;  //纳秒
}
```

同时`pselect`函数增加了一个参数`sigmask` 该参数是一个指向信号掩码的指针。增加该参数的目的在于：避免在阻塞于`select`函数时漏掉某些系统信号。当`sigmask`值位`NULL`时，`pselect`与`select`行为一致。

`pselect`在执行时会阻塞`sigmask`标明的相应信号，并保存这些信号的值，当`pselect`函数返回时该信号被恢复，程序能够正常处理这些信号值。


## poll 函数
`poll`与`select`函数的功能基本相同，但是在处理流设备(来源于目标信息对)时，`poll`函数能够提供额外的信息。
```c
#include <poll.h>

int poll(struct pollfd* fdarry, unsigned long nfds, int timeout);
```
参数`fdarry`是一个指向`pollfd`结构数组的指针，数组中的每一个元素标识一个描述符，该结构的定义如下：
```c
struct pollfd{
   int fd;
   short events;
   short revents;
}
```
成员fd用来标识是否检测对应的描述符状态(是对应描述符的值),当fd的值为负数时说明不检测；成员events指定测试的条件，如可读、可写、异常等；成员revents负责返回描述符的状态；成员events、revents的取值范围如下：
```
POLLIN         普通或优先级带数据可读
POLLRDNORM     普通数据可读
POLLRDBAND     优先级带数据可读
POLLPRI        高优先级数据可读
------------------------------------------------
POLLOUT        普通数据可写
POLLWRNORM     普通数据可写
POLLWRBAND     优先级带数据可写
------------------------------------------------
POLLERR        发生错误
POLLHUP        发生挂起
POLLNVAL       描述符不是一个打开的文件
```
以上取值，成员`events`的输入只能在读、写两部分中取值，成员`revents`可返回全部类型的值

对于描述中的普通、优先等数据的修饰词应当如何理解？POSIX做了相应的规定
1. 所有的正规TCP和所有的UDP数据被认为是普通数据；
2. TCP的带外数据被认为是优先级带数据 <font color=#FFOOO>带外数据指的是连接的某段发生了重要的事件，希望迅速通知对端承载这些信息的就是带外数据，但是带外数据不会新建连接</font>；
3. 当TCP连接的读半部关闭时，也认为是普通数据，随后的读操作将会返回0；
4. TCP连接的错误即可认为是普通数据也可认为是错误，随后的读操作均会返回-1，并设置errno为对应的值；
5. 监听socket上的新连接即可视为普通数据也可视为优先级数据；
6. 非阻塞式`connect`的完成被认为是对应的socket可写；


参数`nfds`指定了`fdarry`数组中的元素个数
参数`timeout`指定了`poll`函数在返回前的等待时间，取值范围如下：
```
INFTIM      永远等待直到有描述符就绪
0           立即返回，不阻塞
>0          等待指定的时长(单位为毫秒)
```

## 总结
从上述部分可以看出，`select`与`poll`存在一下几点的不同：

1. `select`使用的是定长数组(长度为1024)，而`poll`使用自定义的结构体数组，长度没有上限；
2. `select`在单进程中最多支持1024个文件描述符，`poll`理论上没有上限(示例程序中设定了MAX_OPEN)长度的上限；
3. `select`需要手动去清除`fd_set`，但是`poll`中用户修改的是pollfd结构的events，系统修改的是revents字段，代码更加简洁；
4. 超时时间结构体、时间精度不同，`poll`更简单；
5. `poll`可以监听更多的事件(见events、revents的取值范围)，`select`就只有读、写、异常三种；


## 习题
1. 我们说过一个描述符集合可以C语言的赋值语句赋给另一个描述符集。如果描述符集是一个整形数组，那么这如何做到？
   
   
   在系统中，描述符集合是一个结构体，该结构体的声明如下所示
   ```c
   typedef struct{
   #ifdef __USE_XOPEN
       __fd_mask fds_bits[];
   #define __FDS_BITS(set) ((set)->fd_bits)
   #else
       __fd_mask __fds_bits[__FD_SETSIZE / __NFDBITS];
   #define __FDS_BITS(set) ((set)->__fds_bits)
   #endif
   } fd_set;
   ```
   在c语言中，两个结构之间是可以互相通过等号赋值的，但是数组不可以。如果需要用数组对另一个数组进行赋值，则需要使用`memcpy`函数或者将数组封装在结构体中

2. 6.3节讨论`select`返回可写条件时，为什么必须限定socket为非阻塞才可以说一次写操作将返回一个正值？
   
   这是因为当socket是阻塞时，如果要写入的数据大于发送缓冲区的最大标识，那么程序将会阻塞在`write`调用上，直到发送缓冲区的空闲空间大于要写入的数据大小为止。
   
   其他事项可以参考[连接](https://blog.csdn.net/analogous_love/article/details/118998472)中的说明