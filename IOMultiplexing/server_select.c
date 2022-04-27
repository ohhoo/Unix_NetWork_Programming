#include "unp.h"

//该程序主要借助select实现一个单线程的TCP服务器程序
int main(int argc, char* argv[]){
    int i, maxi, maxfd, listenfd, connfd, sockfd;
    int nready, client[FD_SETSIZE];
    ssize_t n;
    fd_set rset, allset;
    char buf[MAXLINE];
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    //监听socket
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    //设置服务器地址、端口
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;

    servaddr.sin_port = htons(SERV_PORT);//需要执行字节序的转换
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    //将servaddr绑定至监听socket
    Bind(listenfd, (SA*)&servaddr, sizeof(servaddr));

    //将listenfd对应的socket设置为监听socket
    Listen(listenfd, LISTENQ);

    maxfd = listenfd;
    maxi = -1;
    for(i = 0; i < FD_SETSIZE; i++)
        client[i] = -1;
    //初始化描述符集合
    FD_ZERO(&allset);
    //打开listenfd对应的位
    FD_SET(listenfd, &allset);

    for( ; ; ){
        rset = allset;
        //Select阻塞于监听socket及其之前的描述符上，等待这些描述符就绪
        nready = Select(maxfd + 1, &rset, NULL, NULL, NULL);

        //listenfd就绪，说明有新的连接到来
        if(FD_ISSET(listenfd, &rset)){
            clilen = sizeof(cliaddr);

            //接收连接，返回新的socket描述符
            connfd = Accept(listenfd, (SA*)&cliaddr, &clilen);

            //将新的连接对应的socket描述符存储在client数组中，client最多存储1024个描述符
            for(i = 0; i < FD_SETSIZE; i++){
                if(client[i] < 0){
                    client[i] = connfd;
                    break;
                }
            }
            if(i == FD_SETSIZE)
                err_quit("too many connect");

            //打开新建立的连接的socket对应的位
            FD_SET(connfd, &allset);
            
            //更改Select将要监视的描述符范围
            if(connfd > maxfd)
                maxfd = connfd;
            
            //更新描述符在client数组内的最大位置
            if(i > maxi)
                maxi = i;

            if(--nready <= 0)//说明没有更多的描述符就绪了，重新开始循环等待下一次连接/数据到来
                continue;
            
        }

        //在目前监测的描述符范围内检查哪些描述符就绪
        for(i = 0; i <= maxi; i++){
            
            //说明当前位置没有对应的描述符
            if(sockfd = client[i] < 0)
                continue;
            
            //检查当前位置的描述符是否是就绪的，如果是则进行处理
            if(FD_ISSET(sockfd, &rset)){
                //从连接对应的socket中读取数据
                if((n = Read(sockfd, buf, MAXLINE)) == 0){//如果读取到EOF
                //在进行读取时可能会阻塞在此处，即如果需要等待客户端输入换行或者EOF才会返回
                //那么就会导致拒绝服务攻击
                //此处可能存在的问题提供给我们的经验就是：服务器在处理多个客户端请求时绝对不能阻塞于
                //只与单个客户相关的函数调用
                    Close(sockfd);//关闭连接
                    FD_CLR(sockfd, &allset);//将描述符从allset中删除
                    client[i] = -1;//将client中该描述符对应位置置为未使用状态
                }
                else
                //写入
                    Writen(sockfd, buf, n);
                
                if(--nready <= 0)//如果没有更多的就绪描述符则进入上一层的for循环中等待连接/数据到来
                    break;
            }
        }
    }
    return 0;
}
