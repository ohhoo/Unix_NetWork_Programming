# 网络通信程序基本的工作模型
进行通信的双方被分为服务器、客户端两类，其区别在于：服务器通常是被动地接受连接，而客户端通常是连接建立的发起者。
<br></br>
**通信双方进行的相应操作(以时间获取服务程序为例进行说明)**<br></br>
服务端:
1. **创建套接字，调用`socket(family,type,protocol)`完成**<br></br>
   <font color=#FF000>family参数指定了网络地址格式，取值包括常量</font>：<br></br>
   <font color=blue>`AF_APPLETALK`  Apple Computer Inc. Appletalk 网络</font><br></br>
   <font color=blue>`AF_INET6`  Internet IPv6 和 IPv4 系列</font><br></br>
   <font color=blue>`AF_INET` 仅 Internet IPv4 系列</font><br></br>
   <font color=blue>`AF_PUP` Xerox Corporation PUP internet</font><br></br>
   <font color=blue>`AF_UNIX` UNIX 文件系统</font><br></br>
   <font color=#FF000>type参数指定了socket的类型，取值包括</font>：<br></br>
   <font color=blue>`SOCK_STREAM` 面向连接，对数据传输进行保障，基于TCP</font><br></br>
   <font color=blue>`SOCK_DGRAM` 面向消息，无数据传输保障，基于UDP</font><br></br>
   <font color=blue>`SOCK_ROW` 原始socket，用于处理普通socket无法处理的ICMP、IGMP报文</font><br></br>
   <font color=#FF000>protocol参数指定了使用的网络协议，通常为0，表示使用socket对应的默认网络协议</font>
   <br></br>
   `socket(family,type,protocol)`会返回一个整数值用以标识创建的socket<br></br>
2.  **将IP、端口信息绑定至套接字，调用`bind(sockfd,name,namelen)`完成**<br></br>
   <font color=#FF000>sockfd参数是socket的handle(一个整数)，由此句柄来识别一个socket</font><br></br>
   <font color=#FF000>name参数是包含了要绑定到的socket的地址类型、IP地址、端口的协议地址结构体指针</font><br></br>
   <font color=#FF000>namelen参数为name参数的长度</font><br></br>
3. **将socket设置为监听socket，调用`listen(sockfd,backlog)`完成**<br></br>
   <font color=#FF000>sockfd参数为socket的标识，是一个整数</font><br></br>
   <font color=#FF000>backlog参数是等待连接队列中所允许的最大连接数目</font><br></br>
   来自客户的外来连接会在监听socket上被内核接受，监听socket是唯一的<br></br>
4. **当客户连接到来时，接收连接并进行数据写入操作。连接的接受通过调用`accept(s,from,fromlen)`完成，写入数据通过调用`write(s,buf,sizeof buf)`完成**<br></br>
   <font color=#FF000>s参数为监听socket的标识</font><br></br>
   <font color=#FF000>from参数用来返回已连接的客户端的协议地址结构体指针，如果对客户端的协议地址不感兴趣可以传入null指针</font><br></br>
   <font color=#FF000>fromlen参数为from参数的长度</font><br></br>
   如果连接建立成功则`accept()`返回一个非负的描述符(整数)，该描述符标识了一个新的socket，否则返回-1<br></br>



   
   
