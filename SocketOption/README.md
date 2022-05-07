# socket 选项
socket选项将会影响到建立连接双方能够进行的操作类型、连接的相关参数。对socket进行设置的方法有：

1. `getsockopt`函数与`setsockopt`函数；
2. `fcntl`函数；
3. `ioctl`函数；
4. `fcntl`函数；


## `getsockopt` 与 `setsockopt` 函数
这两个函数仅能作用于socket(即操作时必须指定对应的socket描述符)，函数的声明如下：
```c
#include <sys/socket.h>

int getsockopt(int sockfd, int level, int optname, void* optval, socklen_t* optlen);

int setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen);
```
两个函数的返回值相同：执行成功返回0，出错返回-1；两个函数的公共参数`sockfd`标明了socket的描述符；`level`参数标明了系统中解释选项的代码或为通用socket代码或为某个特定于协议的代码(如IPv4、IPv6、TCP或SCTP)、`optname`参数标明了选项名(可选的选项名在书中的表格给出；不同的`optname`将会对应不同的`level`;

函数`getsockopt`的参数`optval`、`optlen`都是值-结果类型的参数，将会分别存储获取到的socket选项值以及选项值的长度

函数`setsockopt`的参数`optval`、`optlen`将会根据参数的值分别对socket选项值、选项值的长度进行设置；

## 通用socket选项

通用的socket选项是协议无关的(由内核中的协议无关的代码进行处理)

### `SO_BROADCAST`
本选项开启/禁止进程发送广播消息的能力。(注意：只有数据报socket才能够支持广播，并且网络也必须支持广播消息)

### `SO_DEBUG`
开启本选项后内核将为该socket在发送、接收分组时保留详细的追踪消息，这些消息可以使用`trpt`程序进行检查。(该选项仅由TCP协议支持，因为TCP协议需要保证传输的质量)

### `SO_DONTROUTE`
该选项规定发送的分组将会绕过底层协议的正常路由机制。路由守护进程会使用该选项将分组强制从某个特定接口送出(通常在路由表不正确的情况下进行该操作)

### `SO_ERROR`
该选项用于在发生错误的情况下设置socket的`so_error`变量，同时该选项只能获取不能设置。

### `SO_KEEPALIVE`
该选项设置后会在socket长时间没有数据交换的情况下自动发送保活探测分节(keep-alive probe)，对端必须对该分节进行回复。

### `SO_LINGER`
该选项指定了对于面向连接的协议，`close`函数将会如何进行关闭的动作。默认的操作是`close`函数立即返回，但是如果由数据残留在socket发送缓冲区内，系统将试着把这些数据发送至对端，`SO_LINGER`选项使得我们可以改变这个默认的操作，改变后的操作将由结构体`linger`中成员的值进行控制。

### `SO_OOBINLINE`
本选项开启时，带外数据将会被留在正常的输入队列中，这种情况下接收函数的`MSG_OOB`标志将不能用来读带外数据。

### `SO_RCVBUF` 与 `SO_SNDBUF`
这两个选项用来设置接收缓冲区、发送缓冲区的大小。需要注意的是TCP连接缓冲区大小的设置需要在连接建立之前，这就意味着对于客户端来说需要在调用`connect`函数之前设置缓冲区大小，而在服务端则要在调用`listen`函数设置socket为监听socket之前对缓冲区大小进行设置。且缓冲区的大小与MSS(最大报文长度)相关。


### `SO_RCVLOWAT` 与 `SO_SNDLOWAT`
这两个选项用于修改接收与发送低水位标记的值。两个标记值由`select`函数使用


### `SO_RCVTIMEO` 与 `SO_SNDTIMEO`
这两个选项用于给发送、接收操作设置一个超时值。

发送超时将会影响`read`、`readv`、`recv`、`recvfrom`、`recvmsg`

接收超时将会影响`write`、`writer`、`send`、`sendto`、`sendmsg`

### `SO_REUSEADDR`
该选项允许多个不同的ip地址使用相同的端口，具体可分为以下四种情况：
1. 允许启动一个监听服务器并绑定在众所周知端口(0~1023)上，即使之前建立的使用该端口的连接仍然存在。如下所述：
   
   ```   
   a. 启动一个监听服务器；
   b. 连接请求到达，派生一个子进程处理客户请求；
   c. 监听服务器终止，但派生的子进程仍在连接上处理客户请求；
   d. 监听服务器重启；
   ```
如果此时监听服务器没有设置`SO_REUSEADDR`选项，那么重启时的`bind`操作将会失败，但是如果在`bind`之前设置了`SO_REUSEADDR`选项，就可以正常启动。<font color=#FF00>对于TCP服务器程序，建议将监听socket都设置该选项</font>

2. 允许在同一端口上启动同一服务器的多个实例，只要每个实例捆绑不同的IP地址(在某些系统上，为了安全起见，不允许重用绑定在通配地址(0.0.0.0)上的端口)
3. 允许单个进程捆绑同一端口到多个socket上，但每次捆绑时的IP地址都要不同
4. 该选项同时允许重复的捆绑，即IP地址与端口号都相同，但是需要传输协议支持这一绑定形式。通常可以在UDP协议中开启该选项。


### `SO_REUSEPORT`

该选项随多播特性引入，主要起到以下两点作用：

1. 允许完全重复的捆绑，但是所有绑定至该IP、端口的socket都必须设置该选项
2. 如果被捆绑的IP地址是多播地址，那么`SO_REUSEPORT`与`SO_REUSEADDR`作用相同


### `SO_TYPE`
该选项返回socket的类型

### `SO_USELOOPBACK`
该选项仅用于路由域(AF_ROUTE)的socket，选项开启时相应的socket会接收在其上发送的任何数据报的副本。



## TCP socket选项
TCP有两个socket选项，他们的级别为`IPPROTO_TCP`

### `TCP_MAXSEG`
该选项用于获取或者设置TCP连接的最大分节大小(MSS)，该选项获取到的值在连接不同阶段是不同的。在连接建立之前获取到的值是默认值，在连接建立后该值是由对端的SYN分节通告的MSS值。而且获取到的MSS值并不一定是实际能使用的大小，可能由于添加了某些选项导致实际能够使用的值小于MSS。


### `TCP_NODELAY`
该选项用于关闭默认打开的`Nagle`算法。该算法是为了减少广域网上的小分组的数目，该算法的规则如下：

1. 包长度达到MSS，则允许发送
2. 包含FIN，允许发送
3. 设置了`TCP_NODELAY`选项，允许发送
4. 发生了超时，则立即发送 


## `fcntl`函数
该函数可以执行各种描述符的控制操作，函数的声明如下：
```c
#include <fcntl>

int fcntl(int fd, int cmd, ...)

```
函数执行成功的返回值取决于参数`cmd`，执行出错则返回-1。其中参数`fd`是将要执行操作的描述符，`cmd`参数是要执行的操作，取值在以下说明的命令中选择。

`fcntl`函数可以对各种描述符进行操作，涉及到网络编程的有：

1. 非阻塞式I/O。通过使用`F_SETFL`命令设置`O_NONBLOCK`文件状态标识，把一个socket设置为阻塞型。
2. 信号驱动式I/O。通过使用`F_SETFL`命令，设置`O_ASYNC`文件状态标识，把一个socket设置为状态改变会引起内核产生`SIGIO`信号的状态。
3. `F_SETOWN`命令允许指定用于接收`SIGIO`(在信号驱动式I/O模式下产生)、`SIGURG`(带外数据到达时产生)信号的socket属主(进程或进程组ID)
4. `F_GETOWN`命令返回socket当前的属主
