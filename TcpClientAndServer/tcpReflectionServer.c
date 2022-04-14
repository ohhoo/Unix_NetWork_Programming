#include "unp.h"

int main(int argc, char* argv[]){
    int listenfd, connfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0); //创建监听socket

    bzero(&servaddr, sizeof(servaddr));
    //为监听socket指定相应的协议、IP、端口信息
    servaddr.sin_family = AF_INET;  
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    //将协议地址绑定至监听socket
    Bind(listenfd, (SA*)&servaddr, sizeof(servaddr));
    //打开监听socket的接受连接功能，并设定连接队列长度
    Listen(listenfd, LISTENQ);

    for(;;){
        clilen = sizeof(cliaddr);
        //接收已完成的连接，生成新的socket
        connfd = Accept(listenfd, (SA*)&cliaddr, &clilen);
        //accept新创建的socket使用与监听socket相同的端口号
        //但是连接是由服务器及客户端的socket共同确定，端口号只起到标识应用的作用。
        char hello[] = "Welcome";
        write(connfd, hello, sizeof(hello));
        if( (childpid = Fork()) == 0 ){//进入子进程
            Close(listenfd);//关闭监听socket(将listenfd的引用计数减一)
            str_echo(connfd);//回显、处理
            exit(0);//退出子进程，同时会关闭所有打开的描述符(connfd的引用计数会减一)
        }
        Close(connfd);//关闭连接socket
    }
}