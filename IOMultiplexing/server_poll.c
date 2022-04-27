#include "unp.h"

#define OPEN_MAX 256
//使用Poll函数实现了一个单线程的tcp 回显服务器
int main(int argc, char* argv[]){
    int i, maxi, listenfd, connfd, sockfd;
    int nready;
    ssize_t n;
    char buf[MAXLINE];
    socklen_t clilen;
    struct pollfd client[OPEN_MAX];
    struct sockaddr_in cliaddr, servaddr;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    Bind(listenfd, (SA*)&servaddr, sizeof(servaddr));

    Listen(listenfd, LISTENQ);

    //初始化poll所看管的pollfd数组
    client[0].fd = listenfd;
    //设置要等待的状态
    client[0].events = POLLRDNORM;//对于连接到达一般认为是普通数据，但也可是优先级数据
    //fd成员设置为-1表示不监听
    for(i = 0;i < OPEN_MAX;i++)
        client[i].fd = -1;
    
    maxi = 0;

    for(;;){
        //程序阻塞于Poll调用
        nready = Poll(client, maxi + 1, INFTIM);
        
        //Poll返回，带来的消息标明连接到达
        if(client[0].revents & POLLRDNORM){
            clilen = sizeof(cliaddr);
            //接收连接
            connfd = Accept(listenfd, (SA*)&cliaddr, &clilen);

            //在Poll看管的队列中添加新建立的连接进行检测
            for(i = 1;i < OPEN_MAX;i++){
                if(client[i].fd < 0){
                    client[i].fd = connfd;
                    break;
                }
            }

            if(i == OPEN_MAX)
                err_quit("too many clients");
            //新建立连接socket等待的状态为普通数据可写
            client[i].events = POLLRDNORM;

            //修改Poll函数需要看管的范围
            if(i > maxi)
                maxi = i;
            //如果没有其他就绪的时间需要处理，就重新开始阻塞于Poll进行等待
            if(--nready <= 0)
                continue;
        }

        //非连接到达，而是其他描述符的等待条件就绪
        for(i = 1; i <= maxi;i++){
            if((sockfd = client[i].fd) < 0)
                continue;
            
            //描述符可读或出错
            if(client[i].revents & (POLLRDNORM | POLLERR)){
                //先读取，如果有错就处理
                if((n = read(sockfd, buf, MAXLINE)) < 0){
                    if(errno == ECONNRESET){
                        Close(sockfd);
                        client[i].fd = -1;
                    }
                    else
                        err_sys("read error");
                }
                else if(n == 0){//客户端关闭连接信号EOF
                    Close(sockfd);
                    client[i].fd = -1;
                }
                else//读取成功就写入
                    Writen(sockfd, buf, n);

                if(--nready <= 0)
                    break;
            }
        }
    }

}